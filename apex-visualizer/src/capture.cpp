#include "apex/capture.hpp"
#include "apex/log.hpp"
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <cwctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>
namespace apex {
namespace {
template<class T>struct Com{T* p=nullptr;Com()=default;Com(const Com&)=delete;Com& operator=(const Com&)=delete;~Com(){if(p)p->Release();}T** put(){return &p;}T* operator->()const{return p;}};
void check(HRESULT h,const char* stage){if(FAILED(h)){std::ostringstream s;s<<stage<<" failed (HRESULT 0x"<<std::hex<<static_cast<unsigned>(h)<<")";throw std::runtime_error(s.str());}}
struct Media{bool com=false,mf=false;Media(){auto h=CoInitializeEx(nullptr,COINIT_MULTITHREADED);check(h,"COM capture initialization");com=true;h=MFStartup(MF_VERSION);if(FAILED(h)){CoUninitialize();com=false;check(h,"Media Foundation startup");}mf=true;}~Media(){if(mf)MFShutdown();if(com)CoUninitialize();}};
using Clock=std::chrono::steady_clock;
}
CaptureSource::~CaptureSource(){stop();}
CaptureSnapshot CaptureSource::snapshot()const{std::lock_guard<std::mutex> lock(mutex_);return state_;}
void CaptureSource::stop(){auto apartment=CoInitializeEx(nullptr,COINIT_MULTITHREADED);struct Apartment{HRESULT h;~Apartment(){if(SUCCEEDED(h))CoUninitialize();}} apartmentCleanup{apartment};stop_=true;IMFMediaSource* source=nullptr;{std::lock_guard<std::mutex> lock(mutex_);source=source_;if(source)source->AddRef();}if(source){source->Shutdown();source->Release();}if(worker_.joinable())worker_.join();}
void CaptureSource::start(const std::filesystem::path& file){stop();stop_=false;{std::lock_guard<std::mutex> lock(mutex_);state_=CaptureSnapshot{};state_.running=true;state_.state="starting";state_.message="Opening external Media Foundation source";state_.source=file.empty()?"OBS Virtual Camera":file.u8string();}try{worker_=std::thread([this,file]{run(file);});}catch(...){std::lock_guard<std::mutex> lock(mutex_);state_.running=false;state_.state="failed";state_.message="Capture worker could not start";throw;}}
void CaptureSource::run(std::filesystem::path file)noexcept{
 try{
  Media media;struct ReleaseSource{std::function<void()> fn;~ReleaseSource(){fn();}} releaseSource{[this]{IMFMediaSource* release=nullptr;{std::lock_guard<std::mutex> lock(mutex_);release=source_;source_=nullptr;}if(release){release->Shutdown();release->Release();}}};Com<IMFSourceReader> reader;Com<IMFMediaSource> owned;Com<IMFAttributes> attributes;check(MFCreateAttributes(attributes.put(),4),"Capture attributes");check(attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING,TRUE),"RGB conversion enable");
  if(file.empty()){
   Com<IMFAttributes> filter;check(MFCreateAttributes(filter.put(),1),"Camera enumeration attributes");check(filter->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID),"Camera source type");IMFActivate** devices=nullptr;UINT32 count=0;check(MFEnumDeviceSources(filter.p,&devices,&count),"Enumerate OBS virtual camera");
   struct Devices{IMFActivate** p;UINT32 n;~Devices(){for(UINT32 i=0;i<n;++i)p[i]->Release();CoTaskMemFree(p);}} cleanup{devices,count};
   for(UINT32 i=0;i<count;++i){wchar_t* raw=nullptr;UINT32 length=0;if(SUCCEEDED(devices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,&raw,&length))){std::wstring name(raw,length);CoTaskMemFree(raw);for(auto& c:name)c=static_cast<wchar_t>(std::towlower(c));if(name.find(L"obs")!=std::wstring::npos&&name.find(L"virtual")!=std::wstring::npos){check(devices[i]->ActivateObject(__uuidof(IMFMediaSource),reinterpret_cast<void**>(owned.put())),"Open OBS virtual camera");break;}}}
   if(!owned.p)throw std::runtime_error("OBS Virtual Camera not found. Start OBS Virtual Camera using Display/Window capture first.");
   {std::lock_guard<std::mutex> lock(mutex_);source_=owned.p;source_->AddRef();}
   check(MFCreateSourceReaderFromMediaSource(owned.p,attributes.p,reader.put()),"Create OBS source reader");
  }else {if(!std::filesystem::is_regular_file(file))throw std::runtime_error("Recording file not found");check(MFCreateSourceReaderFromURL(file.c_str(),attributes.p,reader.put()),"Open local recording");}
  check(reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS,FALSE),"Disable unused streams");check(reader->SetStreamSelection(MF_SOURCE_READER_FIRST_VIDEO_STREAM,TRUE),"Select video only");Com<IMFMediaType> desired;check(MFCreateMediaType(desired.put()),"RGB media type");check(desired->SetGUID(MF_MT_MAJOR_TYPE,MFMediaType_Video),"Video type");check(desired->SetGUID(MF_MT_SUBTYPE,MFVideoFormat_RGB32),"RGB32 subtype");check(reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM,nullptr,desired.p),"Negotiate RGB32 video");
  auto start=Clock::now();std::shared_ptr<const CapturedFrame> previous;long long firstTimestamp=-1;bool ended=false;
  while(!stop_){DWORD flags=0,stream=0;LONGLONG timestamp=0;Com<IMFSample> sample;auto readStart=Clock::now();HRESULT result=reader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM,0,&stream,&flags,&timestamp,sample.put());if(stop_)break;check(result,"Read capture frame");if(flags&MF_SOURCE_READERF_ENDOFSTREAM){ended=true;break;}if(!sample.p)continue;
   Com<IMFMediaType> type;check(reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM,type.put()),"Read frame format");UINT32 width=0,height=0;check(MFGetAttributeSize(type.p,MF_MT_FRAME_SIZE,&width,&height),"Read frame dimensions");if(!width||!height||width>4096||height>2160)throw std::runtime_error("Capture bound is 4096x2160; reduce OBS output resolution");
   unsigned cropX=0,cropY=0,displayWidth=width,displayHeight=height;MFVideoArea area{};UINT32 areaBytes=0;
   if(SUCCEEDED(type->GetBlob(MF_MT_MINIMUM_DISPLAY_APERTURE,reinterpret_cast<UINT8*>(&area),sizeof(area),&areaBytes))&&areaBytes==sizeof(area)&&area.OffsetX.value>=0&&area.OffsetY.value>=0&&area.Area.cx>0&&area.Area.cy>0){auto x=static_cast<unsigned>(area.OffsetX.value),y=static_cast<unsigned>(area.OffsetY.value);auto w=static_cast<unsigned>(area.Area.cx),h=static_cast<unsigned>(area.Area.cy);if(x<=width&&y<=height&&w<=width-x&&h<=height-y){cropX=x;cropY=y;displayWidth=w;displayHeight=h;}}
   auto frame=std::make_shared<CapturedFrame>();frame->width=displayWidth;frame->height=displayHeight;frame->sourceTimestamp100ns=timestamp;frame->readWaitMs=std::chrono::duration<double,std::milli>(Clock::now()-readStart).count();frame->rgba.resize(size_t(displayWidth)*displayHeight*4);Com<IMFMediaBuffer> buffer;check(sample->ConvertToContiguousBuffer(buffer.put()),"Contiguous video buffer");Com<IMF2DBuffer> two;BYTE* pixels=nullptr;LONG pitch=0;DWORD length=0;bool locked2d=false;
   if(SUCCEEDED(buffer->QueryInterface(__uuidof(IMF2DBuffer),reinterpret_cast<void**>(two.put())))){check(two->Lock2D(&pixels,&pitch),"Lock 2D frame");locked2d=true;}
   else {DWORD max=0;check(buffer->Lock(&pixels,&max,&length),"Lock video frame");UINT32 stride=0;if(SUCCEEDED(type->GetUINT32(MF_MT_DEFAULT_STRIDE,&stride)))pitch=static_cast<LONG>(stride);else pitch=static_cast<LONG>(width*4);if(std::abs(static_cast<long long>(pitch))*height>length){buffer->Unlock();throw std::runtime_error("Video buffer stride/length mismatch");}if(pitch<0)pixels+=size_t(-pitch)*(height-1);}
   if(std::abs(static_cast<long long>(pitch))<static_cast<long long>(width)*4){if(locked2d)two->Unlock2D();else buffer->Unlock();throw std::runtime_error("Video stride is shorter than one row");}
   for(UINT32 y=0;y<displayHeight;++y){const BYTE* row=pixels+static_cast<ptrdiff_t>(y+cropY)*pitch+size_t(cropX)*4;auto* out=frame->rgba.data()+size_t(y)*displayWidth*4;for(UINT32 x=0;x<displayWidth;++x){out[x*4]=row[x*4+2];out[x*4+1]=row[x*4+1];out[x*4+2]=row[x*4];out[x*4+3]=255;}}
   if(locked2d)two->Unlock2D();else buffer->Unlock();auto stats=frameStatistics(frame->rgba,displayWidth,displayHeight,previous&&previous->width==displayWidth&&previous->height==displayHeight?&previous->rgba:nullptr);previous=frame;
   if(!file.empty()){if(firstTimestamp<0)firstTimestamp=timestamp;auto target=start+std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>((timestamp-firstTimestamp)/10000000.));while(!stop_&&Clock::now()<target)std::this_thread::sleep_for(std::chrono::milliseconds(2));}
   {std::lock_guard<std::mutex> lock(mutex_);frame->sequence=++state_.frames;state_.width=displayWidth;state_.height=displayHeight;state_.readWaitMs=frame->readWaitMs;state_.fps=state_.frames/std::max(.001,std::chrono::duration<double>(Clock::now()-start).count());state_.statistics=stats;state_.frame=frame;state_.state="capturing";state_.message="External pixels received; semantic analysis requires a compatible vision model";}
  }
  {std::lock_guard<std::mutex> lock(mutex_);state_.running=false;state_.state=ended?"end_of_recording":"stopped";state_.message=ended?"Recording completed; last real frame retained":"Capture stopped";}
 }catch(const std::exception& e){Log::write("WARN",std::string("Capture: ")+e.what());std::lock_guard<std::mutex> lock(mutex_);state_.running=false;state_.state=stop_?"stopped":"unavailable";state_.message=stop_?"Capture stopped":e.what();}
}
}
