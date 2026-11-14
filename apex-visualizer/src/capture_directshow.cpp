#include "apex/capture_directshow.hpp"
#include "apex/log.hpp"
#include <windows.h>
#include <dshow.h>
#include <dvdmedia.h>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstring>
#include <sstream>
#include <stdexcept>
namespace apex {namespace {
template<class T>struct Com {T* p=nullptr;Com()=default;Com(const Com&)=delete;Com& operator=(const Com&)=delete;~Com(){if(p)p->Release();}T** put(){return &p;}T* operator->()const{return p;}};
void check(HRESULT h,const char* stage){if(FAILED(h)){std::ostringstream out;out<<"DirectShow "<<stage<<" failed (0x"<<std::hex<<static_cast<unsigned>(h)<<")";throw std::runtime_error(out.str());}}
std::string utf8(const std::wstring& s){if(s.empty())return {};int n=WideCharToMultiByte(CP_UTF8,0,s.data(),static_cast<int>(s.size()),nullptr,0,nullptr,nullptr);std::string out(n,0);if(n)WideCharToMultiByte(CP_UTF8,0,s.data(),static_cast<int>(s.size()),out.data(),n,nullptr,nullptr);return out;}
void findPin(IBaseFilter* filter,PIN_DIRECTION direction,IPin** out){Com<IEnumPins> pins;check(filter->EnumPins(pins.put()),"enumerate filter pins");while(true){Com<IPin> pin;ULONG got=0;HRESULT h=pins->Next(1,pin.put(),&got);if(h==S_FALSE)break;check(h,"read filter pin");PIN_DIRECTION actual;check(pin->QueryDirection(&actual),"query pin direction");if(actual==direction){*out=pin.p;(*out)->AddRef();return;}}throw std::runtime_error("DirectShow required filter pin not found");}
const CLSID grabberClass={0xc1f400a0,0x3f08,0x11d3,{0x9f,0x0b,0x00,0x60,0x08,0x03,0x9e,0x37}};
const CLSID nullRendererClass={0xc1f400a4,0x3f08,0x11d3,{0x9f,0x0b,0x00,0x60,0x08,0x03,0x9e,0x37}};
const IID grabberId={0x6b652fff,0x11fe,0x4fce,{0x92,0xad,0x02,0x66,0xb5,0xd7,0xc7,0x8f}};
const IID callbackId={0x0579154a,0x2b53,0x4994,{0xb0,0xd0,0xe7,0x73,0x14,0x8e,0xff,0x85}};
// The Sample Grabber remains a Windows DirectShow component. Its legacy qedit.h
// is absent from modern SDKs; these are its documented COM vtables, not hooks.
struct GrabberCallback: IUnknown {virtual HRESULT STDMETHODCALLTYPE SampleCB(double,IMediaSample*)=0;virtual HRESULT STDMETHODCALLTYPE BufferCB(double,BYTE*,long)=0;};
struct Grabber: IUnknown {
 virtual HRESULT STDMETHODCALLTYPE SetOneShot(BOOL)=0;
 virtual HRESULT STDMETHODCALLTYPE SetMediaType(const AM_MEDIA_TYPE*)=0;
 virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(AM_MEDIA_TYPE*)=0;
 virtual HRESULT STDMETHODCALLTYPE SetBufferSamples(BOOL)=0;
 virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer(long*,long*)=0;
 virtual HRESULT STDMETHODCALLTYPE GetCurrentSample(IMediaSample**)=0;
 virtual HRESULT STDMETHODCALLTYPE SetCallback(GrabberCallback*,long)=0;
};
struct MediaType{AM_MEDIA_TYPE value{};~MediaType(){if(value.cbFormat)CoTaskMemFree(value.pbFormat);if(value.pUnk)value.pUnk->Release();}};
CameraInventory enumerate(IBaseFilter** selected){
 CameraInventory result;Com<ICreateDevEnum> devices;check(CoCreateInstance(CLSID_SystemDeviceEnum,nullptr,CLSCTX_INPROC_SERVER,IID_ICreateDevEnum,reinterpret_cast<void**>(devices.put())),"create camera enumerator");Com<IEnumMoniker> entries;HRESULT h=devices->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,entries.put(),0);if(h==S_FALSE)return result;check(h,"enumerate video inputs");
 while(true){Com<IMoniker> moniker;ULONG fetched=0;h=entries->Next(1,moniker.put(),&fetched);if(h==S_FALSE)break;check(h,"read camera entry");Com<IPropertyBag> bag;if(FAILED(moniker->BindToStorage(nullptr,nullptr,IID_IPropertyBag,reinterpret_cast<void**>(bag.put()))))continue;
  struct Value{VARIANT v;Value(){VariantInit(&v);}~Value(){VariantClear(&v);}} name;
  if(FAILED(bag->Read(L"FriendlyName",&name.v,nullptr))||name.v.vt!=VT_BSTR)continue;std::string friendly=utf8(std::wstring(name.v.bstrVal,SysStringLen(name.v.bstrVal)));result.names.push_back(friendly);
  if(isObsVirtualCamera(friendly)){result.obsFound=true;if(selected&&!*selected)check(moniker->BindToObject(nullptr,nullptr,IID_IBaseFilter,reinterpret_cast<void**>(selected)),"open OBS Virtual Camera");}
 }
 return result;
}
using Clock=std::chrono::steady_clock;
struct SampleSink final: GrabberCallback {
 std::atomic<ULONG> references{1};std::mutex mutex;std::vector<unsigned char> bytes;std::uint64_t sequence=0;double timestamp=0;size_t required;bool failed=false;Clock::time_point received;
 explicit SampleSink(size_t size):required(size){}
 HRESULT STDMETHODCALLTYPE QueryInterface(REFIID id,void** out)override{if(!out)return E_POINTER;*out=nullptr;if(IsEqualIID(id,IID_IUnknown)||IsEqualIID(id,callbackId)){*out=static_cast<GrabberCallback*>(this);AddRef();return S_OK;}return E_NOINTERFACE;}
 ULONG STDMETHODCALLTYPE AddRef()override{return ++references;}
 ULONG STDMETHODCALLTYPE Release()override{auto n=--references;if(!n)delete this;return n;}
 HRESULT STDMETHODCALLTYPE SampleCB(double,IMediaSample*)override{return E_NOTIMPL;}
 HRESULT STDMETHODCALLTYPE BufferCB(double time,BYTE* data,long size)override{
  try{std::lock_guard<std::mutex> lock(mutex);if(!data||size<0||static_cast<size_t>(size)<required){failed=true;return S_OK;}bytes.resize(required);std::memcpy(bytes.data(),data,required);timestamp=time;received=Clock::now();++sequence;}catch(...){std::lock_guard<std::mutex> lock(mutex);failed=true;}return S_OK;
 }
};
}
bool isObsVirtualCamera(const std::string& name){auto lower=name;for(auto& c:lower)c=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));return lower.find("obs")!=std::string::npos&&lower.find("virtual")!=std::string::npos;}
CameraInventory directShowCameras(){return enumerate(nullptr);}
static void runDirectShow(std::atomic<bool>& stop,const std::function<void(std::shared_ptr<CapturedFrame>)>& publish,const std::function<void(const std::string&)>& status,const std::filesystem::path& recording){
 Com<IBaseFilter> source;Com<IPin> cameraOut;
 Com<IGraphBuilder> graph;check(CoCreateInstance(CLSID_FilterGraph,nullptr,CLSCTX_INPROC_SERVER,IID_IGraphBuilder,reinterpret_cast<void**>(graph.put())),"create filter graph");Com<ICaptureGraphBuilder2> builder;check(CoCreateInstance(CLSID_CaptureGraphBuilder2,nullptr,CLSCTX_INPROC_SERVER,IID_ICaptureGraphBuilder2,reinterpret_cast<void**>(builder.put())),"create capture graph builder");check(builder->SetFiltergraph(graph.p),"set graph");if(recording.empty()){
  auto inventory=enumerate(source.put());std::string list;for(const auto& name:inventory.names){if(!list.empty())list+=", ";list+=name;}Log::write("INFO","DirectShow video inputs ("+std::to_string(inventory.names.size())+"): "+list);
  if(!source.p)throw std::runtime_error("Media Foundation did not find OBS. DirectShow also found no OBS Virtual Camera ("+std::to_string(inventory.names.size())+" inputs: "+(list.empty()?"none":list)+"). Repair the 64-bit OBS Virtual Camera installation, then restart this app.");
  status("OBS Virtual Camera found through DirectShow; opening video stream");check(graph->AddFilter(source.p,L"OBS Virtual Camera"),"add OBS source");
 }else check(graph->AddSourceFilter(recording.c_str(),L"Original recorded fixture",source.put()),"open recorded fixture");
 if(recording.empty()){
  findPin(source.p,PINDIR_OUTPUT,cameraOut.put());Com<IAMStreamConfig> config;check(cameraOut->QueryInterface(IID_IAMStreamConfig,reinterpret_cast<void**>(config.put())),"query native camera pin configuration");
  int count=0,capBytes=0;check(config->GetNumberOfCapabilities(&count,&capBytes),"get native camera capabilities");Log::write("INFO","DirectShow native camera capabilities: count="+std::to_string(count)+" bytes="+std::to_string(capBytes));if(count<0||count>4096||capBytes<1||capBytes>65536)throw std::runtime_error("Invalid camera format-capability bounds");std::vector<BYTE> caps(static_cast<size_t>(capBytes));bool configured=false;
  for(int index=0;index<count;++index){AM_MEDIA_TYPE* rawType=nullptr;HRESULT h=config->GetStreamCaps(index,&rawType,caps.data());if(FAILED(h)||!rawType)continue;struct TypeCleanup{AM_MEDIA_TYPE* p;~TypeCleanup(){if(p->cbFormat)CoTaskMemFree(p->pbFormat);if(p->pUnk)p->pUnk->Release();CoTaskMemFree(p);}} freeType{rawType};
   // Capture native YUY2 directly; do not depend on an implicit RGB converter.
   if(rawType->subtype==MEDIASUBTYPE_YUY2){h=config->SetFormat(rawType);if(SUCCEEDED(h)){configured=true;Log::write("INFO","DirectShow explicitly configured native OBS YUY2 format, cap="+std::to_string(index));break;}Log::write("WARN","DirectShow native YUY2 SetFormat failed: "+std::to_string(static_cast<unsigned>(h)));}
  }
  if(!configured){
   // A fresh official OBS filter has zero advertised formats until OBS has
   // produced/cached a resolution. Its IAMStreamConfig accepts a concrete
   // consumer-supplied format. Set a bounded 720p30 YUY2 format explicitly.
   VIDEOINFOHEADER video{};video.AvgTimePerFrame=333333;video.rcSource={0,0,1280,720};video.rcTarget=video.rcSource;video.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);video.bmiHeader.biWidth=1280;video.bmiHeader.biHeight=720;video.bmiHeader.biPlanes=1;video.bmiHeader.biBitCount=16;video.bmiHeader.biCompression=MAKEFOURCC('Y','U','Y','2');video.bmiHeader.biSizeImage=1280*720*2;video.dwBitRate=1280*720*16*30;
   AM_MEDIA_TYPE fallback{};fallback.majortype=MEDIATYPE_Video;fallback.subtype=MEDIASUBTYPE_YUY2;fallback.bFixedSizeSamples=TRUE;fallback.lSampleSize=video.bmiHeader.biSizeImage;fallback.formattype=FORMAT_VideoInfo;fallback.cbFormat=sizeof(video);fallback.pbFormat=reinterpret_cast<BYTE*>(&video);check(config->SetFormat(&fallback),"set explicit OBS native 1280x720 YUY2 format");Log::write("INFO","DirectShow configured explicit OBS 1280x720 YUY2 fallback (no usable advertised format)");
  }
 }
 Com<IBaseFilter> filter;check(CoCreateInstance(grabberClass,nullptr,CLSCTX_INPROC_SERVER,IID_IBaseFilter,reinterpret_cast<void**>(filter.put())),"create sample grabber");Com<Grabber> grabber;check(filter->QueryInterface(grabberId,reinterpret_cast<void**>(grabber.put())),"get sample grabber interface");AM_MEDIA_TYPE desired{};desired.majortype=MEDIATYPE_Video;desired.subtype=recording.empty()?MEDIASUBTYPE_YUY2:MEDIASUBTYPE_RGB32;desired.formattype=GUID_NULL;check(grabber->SetMediaType(&desired),"request RGB32 conversion");check(grabber->SetOneShot(FALSE),"continuous capture");check(grabber->SetBufferSamples(FALSE),"disable redundant internal frame buffer");check(graph->AddFilter(filter.p,L"App frame receiver"),"add frame receiver");Com<IBaseFilter> renderer;check(CoCreateInstance(nullRendererClass,nullptr,CLSCTX_INPROC_SERVER,IID_IBaseFilter,reinterpret_cast<void**>(renderer.put())),"create null renderer");check(graph->AddFilter(renderer.p,L"No external video window"),"add null renderer");if(recording.empty()){Com<IPin> input,output,renderInput;findPin(filter.p,PINDIR_INPUT,input.put());findPin(filter.p,PINDIR_OUTPUT,output.put());findPin(renderer.p,PINDIR_INPUT,renderInput.put());check(graph->ConnectDirect(cameraOut.p,input.p,nullptr),"connect native OBS pin directly to frame receiver");MediaType outputType;check(grabber->GetConnectedMediaType(&outputType.value),"read configured native frame type");check(graph->ConnectDirect(output.p,renderInput.p,&outputType.value),"connect native frame receiver directly to null renderer");}else check(builder->RenderStream(nullptr,nullptr,source.p,filter.p,renderer.p),"connect recorded probe stream");
 MediaType connected;check(grabber->GetConnectedMediaType(&connected.value),"read connected format");BITMAPINFOHEADER info{};auto& type=connected.value;
 if(type.formattype==FORMAT_VideoInfo&&type.cbFormat>=sizeof(VIDEOINFOHEADER))info=reinterpret_cast<VIDEOINFOHEADER*>(type.pbFormat)->bmiHeader;
 else if(type.formattype==FORMAT_VideoInfo2&&type.cbFormat>=sizeof(VIDEOINFOHEADER2))info=reinterpret_cast<VIDEOINFOHEADER2*>(type.pbFormat)->bmiHeader;
 else throw std::runtime_error("DirectShow connected video format is unsupported");
 const long long height=info.biHeight<0?-static_cast<long long>(info.biHeight):info.biHeight;
 const bool yuy2=recording.empty();
 if((yuy2?(type.subtype!=MEDIASUBTYPE_YUY2||info.biBitCount!=16||info.biWidth%2):(type.subtype!=MEDIASUBTYPE_RGB32||info.biBitCount!=32))||info.biWidth<=0||info.biWidth>4096||height<=0||height>2160)throw std::runtime_error("DirectShow requires native YUY2 camera or RGB32 recording video bounded to 4096x2160; set OBS output to 1280x720");
 const unsigned width=static_cast<unsigned>(info.biWidth),h=static_cast<unsigned>(height);const size_t stride=size_t(width)*(yuy2?2:4);
 if(recording.empty()){Com<IMediaFilter> media;check(graph->QueryInterface(IID_IMediaFilter,reinterpret_cast<void**>(media.put())),"get live graph timing control");check(media->SetSyncSource(nullptr),"disable playback clock for live camera");Log::write("INFO","DirectShow live camera playback clock disabled; OBS paces its own capture samples");}
 Com<SampleSink> sink;sink.p=new SampleSink(stride*h);Com<IMediaControl> control;check(graph->QueryInterface(IID_IMediaControl,reinterpret_cast<void**>(control.put())),"get capture control");
 struct StopGraph{IMediaControl* control;Grabber* grabber;~StopGraph(){control->Stop();grabber->SetCallback(nullptr,1);}} cleanup{control.p,grabber.p};check(grabber->SetCallback(sink.p,1),"set actual-sample callback");// OBS VCamFilter signals its capture thread from Pause(), not Run().
 // Make the Stopped -> Paused -> Running sequence explicit for live sources.
 HRESULT pauseResult=control->Pause();check(pauseResult,"prepare camera (Pause)");
 HRESULT runResult=control->Run();check(runResult,"start video stream (Run)");
 Com<IMediaEvent> events;check(graph->QueryInterface(IID_IMediaEvent,reinterpret_cast<void**>(events.put())),"get stream events");
 OAFilterState graphState=State_Stopped;HRESULT stateResult=control->GetState(0,&graphState);
 std::ostringstream startup;startup<<"DirectShow Pause=0x"<<std::hex<<static_cast<unsigned>(pauseResult)<<" Run=0x"<<static_cast<unsigned>(runResult)<<" GetState=0x"<<static_cast<unsigned>(stateResult)<<std::dec<<" state="<<graphState;Log::write("INFO",startup.str());
 status(recording.empty()?"DirectShow camera prepared and started; waiting for actual OBS samples":"DirectShow recording probe started; waiting for actual file samples");auto lastFrame=Clock::now();std::uint64_t last=0;
 while(!stop){long code=0;LONG_PTR one=0,two=0;while(events->GetEvent(&code,&one,&two,0)==S_OK){events->FreeEventParams(code,one,two);if(code==EC_ERRORABORT||code==EC_STREAM_ERROR_STOPPED||(code==EC_DEVICE_LOST&&two==0)){std::ostringstream message;message<<"DirectShow stream failure event="<<code<<" detail=0x"<<std::hex<<static_cast<unsigned long>(one);throw std::runtime_error(message.str());}}
 std::vector<unsigned char> raw;double timestamp=0;Clock::time_point received;{std::lock_guard<std::mutex> lock(sink->mutex);if(sink->failed)throw std::runtime_error("DirectShow received an invalid frame or could not allocate its bounded frame buffer");if(sink->sequence!=last){last=sink->sequence;timestamp=sink->timestamp;received=sink->received;raw.swap(sink->bytes);}}
  if(raw.empty()){if(Clock::now()-lastFrame>std::chrono::seconds(8)){OAFilterState current=State_Stopped;HRESULT currentResult=control->GetState(0,&current);std::ostringstream message;message<<"DirectShow received no samples for 8 seconds after explicit Pause/Run; callbacks="<<last<<" graph_state="<<current<<" state_hresult=0x"<<std::hex<<static_cast<unsigned>(currentResult)<<" pause_hresult=0x"<<static_cast<unsigned>(pauseResult)<<" run_hresult=0x"<<static_cast<unsigned>(runResult)<<". Check OBS output or whether another camera app receives OBS video.";throw std::runtime_error(message.str());};std::this_thread::sleep_for(std::chrono::milliseconds(5));continue;}
  lastFrame=Clock::now();auto frame=std::make_shared<CapturedFrame>();frame->width=width;frame->height=h;frame->sourceTimestamp100ns=static_cast<long long>(timestamp*10000000.);frame->readWaitMs=std::chrono::duration<double,std::milli>(Clock::now()-received).count();frame->rgba.resize(size_t(width)*h*4);
  if(yuy2)yuy2ToRgba(raw,width,h,frame->rgba);else for(unsigned y=0;y<h;++y){const auto* row=raw.data()+size_t(info.biHeight>0?h-1-y:y)*stride;auto* out=frame->rgba.data()+size_t(y)*stride;for(unsigned x=0;x<width;++x){out[x*4]=row[x*4+2];out[x*4+1]=row[x*4+1];out[x*4+2]=row[x*4];out[x*4+3]=255;}}
  publish(std::move(frame));
 }
}
void runObsDirectShow(std::atomic<bool>& stop,const std::function<void(std::shared_ptr<CapturedFrame>)>& publish,const std::function<void(const std::string&)>& status){runDirectShow(stop,publish,status,{});}
void probeDirectShowRecording(const std::filesystem::path& file,std::atomic<bool>& stop,const std::function<void(std::shared_ptr<CapturedFrame>)>& publish){if(file.empty())throw std::runtime_error("DirectShow recording probe needs an actual local file");runDirectShow(stop,publish,[](const std::string& text){Log::write("INFO",text);},file);}

}
