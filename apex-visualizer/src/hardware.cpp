#include "apex/hardware.hpp"
#include "apex/log.hpp"
#include <algorithm>
#include <cctype>
#include <vector>
#include <cwctype>
#include <cwchar>
#include <cmath>
namespace apex {
namespace {
template<class T>T symbol(HMODULE m,const char* name){return reinterpret_cast<T>(GetProcAddress(m,name));}
std::string normalized(std::string s){for(char& c:s)c=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));return s;}
HMODULE systemLibrary(const wchar_t* name){return LoadLibraryExW(name,nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);}
struct Util {unsigned gpu,memory;};struct Memory {unsigned long long total,free,used;};
}
HardwareMetrics::~HardwareMetrics(){if(processQuery_)PdhCloseQuery(processQuery_);if(initialized_&&shutdown_)shutdown_();if(nvml_)FreeLibrary(nvml_);}
void HardwareMetrics::initialize(const std::string& adapter,LUID luid){
 pidMatch_=L"pid_"+std::to_wstring(GetCurrentProcessId())+L"_";wchar_t text[80]{};swprintf_s(text,80,L"_luid_0x%08x_0x%08x_",static_cast<unsigned>(luid.HighPart),static_cast<unsigned>(luid.LowPart));luidMatch_=text;
 processCounterStatus="Process GPU engine counter unavailable";
 if(PdhOpenQueryW(nullptr,0,&processQuery_)==ERROR_SUCCESS&&PdhAddEnglishCounterW(processQuery_,L"\\GPU Engine(*)\\Utilization Percentage",0,&processCounter_)==ERROR_SUCCESS){pdhPrimed_=PdhCollectQueryData(processQuery_)==ERROR_SUCCESS;processCounterStatus="Process GPU engine counter warming up";}

 // CUDA driver API only: no toolkit/runtime dependency and no context or model is created.
 HMODULE cuda=systemLibrary(L"nvcuda.dll");cudaStatus="CUDA driver unavailable; DX11 remains active";
 if(cuda){using Init=int(__stdcall*)(unsigned);using Count=int(__stdcall*)(int*);using Name=int(__stdcall*)(char*,int,int);using Capability=int(__stdcall*)(int*,int*,int);using Version=int(__stdcall*)(int*);
  auto init=symbol<Init>(cuda,"cuInit");auto count=symbol<Count>(cuda,"cuDeviceGetCount");auto name=symbol<Name>(cuda,"cuDeviceGetName");auto cap=symbol<Capability>(cuda,"cuDeviceComputeCapability");auto version=symbol<Version>(cuda,"cuDriverGetVersion");int n=0,matches=0;
  if(init&&count&&name&&cap&&version&&init(0)==0&&count(&n)==0){for(int i=0;i<n;++i){char value[256]{};if(name(value,256,i)==0&&normalized(value)==normalized(adapter)&&cap(&computeMajor,&computeMinor,i)==0)++matches;}cudaAvailable=matches==1&&version(&cudaDriver)==0;cudaStatus=cudaAvailable?"CUDA driver/device verified; renderer uses DX11 (no CUDA workload)":"CUDA present; selected DXGI adapter not uniquely verified";}
  else cudaStatus="CUDA driver initialization failed; DX11 remains active";
  FreeLibrary(cuda);
 }
 nvml_=systemLibrary(L"nvml.dll");
 if(!nvml_){wchar_t root[32768];DWORD n=GetEnvironmentVariableW(L"ProgramW6432",root,32768);if(n&&n<32768){auto path=std::wstring(root)+L"\\NVIDIA Corporation\\NVSMI\\nvml.dll";nvml_=LoadLibraryExW(path.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);}}
 nvmlStatus="NVIDIA counters unavailable (NVML missing)";
 if(nvml_){using Init=int(__cdecl*)();using Count=int(__cdecl*)(unsigned*);using Handle=int(__cdecl*)(unsigned,void**);using Name=int(__cdecl*)(void*,char*,unsigned);
  auto init=symbol<Init>(nvml_,"nvmlInit_v2");auto count=symbol<Count>(nvml_,"nvmlDeviceGetCount_v2");auto handle=symbol<Handle>(nvml_,"nvmlDeviceGetHandleByIndex_v2");auto name=symbol<Name>(nvml_,"nvmlDeviceGetName");shutdown_=symbol<ShutdownFn>(nvml_,"nvmlShutdown");
  utilizationFn_=symbol<UtilFn>(nvml_,"nvmlDeviceGetUtilizationRates");memoryFn_=symbol<MemoryFn>(nvml_,"nvmlDeviceGetMemoryInfo");unsigned n=0,matches=0;
  if(init&&shutdown_&&count&&handle&&name&&init()==0){initialized_=true;if(count(&n)==0)for(unsigned i=0;i<n;++i){void* d=nullptr;char value[256]{};if(handle(i,&d)==0&&name(d,value,256)==0&&normalized(value)==normalized(adapter)){device_=d;++matches;}}
   if(matches!=1){device_=nullptr;nvmlStatus="NVML adapter match unavailable/ambiguous; counters disabled";}else nvmlStatus="NVML matched adapter; sampling device-wide counters";
  }else nvmlStatus="NVML initialization failed; counters disabled";
 }
 Log::write("INFO",cudaStatus);Log::write("INFO",nvmlStatus);
}
void HardwareMetrics::update(double now){
 if(sampled_>=0&&now-sampled_<.5)return;sampled_=now;utilizationValid=memoryValid=false;
 processUtilizationValid=false;
 if(processQuery_&&processCounter_){auto code=PdhCollectQueryData(processQuery_);if(code==ERROR_SUCCESS&&pdhPrimed_){DWORD bytes=0,count=0;code=PdhGetFormattedCounterArrayW(processCounter_,PDH_FMT_DOUBLE|PDH_FMT_NOCAP100,&bytes,&count,nullptr);
   if(code==PDH_MORE_DATA&&bytes){std::vector<unsigned char> buffer(bytes);auto* items=reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM_W*>(buffer.data());code=PdhGetFormattedCounterArrayW(processCounter_,PDH_FMT_DOUBLE|PDH_FMT_NOCAP100,&bytes,&count,items);if(code==ERROR_SUCCESS){double busiest=0;bool matched=false;for(DWORD i=0;i<count;++i){std::wstring name=items[i].szName?items[i].szName:L"";for(auto& c:name)c=static_cast<wchar_t>(std::towlower(c));if(name.find(pidMatch_)!=0||name.find(luidMatch_)==std::wstring::npos)continue;const auto& value=items[i].FmtValue;if((value.CStatus==PDH_CSTATUS_VALID_DATA||value.CStatus==PDH_CSTATUS_NEW_DATA)&&std::isfinite(value.doubleValue)&&value.doubleValue>=0){busiest=std::max(busiest,value.doubleValue);matched=true;}}
    if(matched){processUtilization=busiest;processUtilizationValid=true;processCounterStatus="PDH matching PID + selected adapter, busiest engine (not sum)";}else processCounterStatus="No valid matching PID/adapter GPU engine samples";}}
  }pdhPrimed_=code==ERROR_SUCCESS||code==PDH_MORE_DATA;}
 if(!device_)return;Util u{};Memory m{};
 if(utilizationFn_&&utilizationFn_(device_,&u)==0&&u.gpu<=100){utilization=u.gpu;utilizationValid=true;}
 if(memoryFn_&&memoryFn_(device_,&m)==0&&m.used<=m.total&&m.total){usedBytes=m.used;totalBytes=m.total;memoryValid=true;}
}
}
