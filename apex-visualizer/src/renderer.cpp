#include "apex/renderer.hpp"
#include "apex/log.hpp"
#include <dxgi1_3.h>
#include <stdexcept>
#include <sstream>
namespace apex {
namespace {
template<class T>void release(T*& p){if(p){p->Release();p=nullptr;}}
void check(HRESULT hr,const char* stage){if(FAILED(hr)){std::ostringstream s;s<<stage<<" failed (HRESULT 0x"<<std::hex<<static_cast<unsigned long>(hr)<<")";throw std::runtime_error(s.str());}}
std::string utf8(const wchar_t* p){int n=WideCharToMultiByte(CP_UTF8,0,p,-1,nullptr,0,nullptr,nullptr);if(n<=1)return "Unknown adapter";std::string s(static_cast<size_t>(n),'\0');WideCharToMultiByte(CP_UTF8,0,p,-1,s.data(),n,nullptr,nullptr);s.pop_back();return s;}
}
Renderer::~Renderer(){cleanup();}
void Renderer::cleanup()noexcept{
 if(context_)context_->ClearState();
 for(auto& q:queries_){release(q.disjoint);release(q.begin);release(q.end);q.pending=false;}
 if(latency_){CloseHandle(latency_);latency_=nullptr;}
 release(memoryAdapter_);release(target_);release(swap_);release(context_);release(device_);
}
void Renderer::initialize(HWND hwnd,bool warp){
 const D3D_FEATURE_LEVEL levels[]={D3D_FEATURE_LEVEL_11_0,D3D_FEATURE_LEVEL_10_1};D3D_FEATURE_LEVEL level{};
 IDXGIFactory1* selector=nullptr;IDXGIAdapter1* chosen=nullptr;SIZE_T largest=0;
 if(!warp&&SUCCEEDED(CreateDXGIFactory1(__uuidof(IDXGIFactory1),reinterpret_cast<void**>(&selector)))){
  for(UINT i=0;;++i){IDXGIAdapter1* candidate=nullptr;HRESULT hr=selector->EnumAdapters1(i,&candidate);if(hr==DXGI_ERROR_NOT_FOUND)break;if(FAILED(hr))break;DXGI_ADAPTER_DESC1 desc{};
   if(SUCCEEDED(candidate->GetDesc1(&desc))&&!(desc.Flags&DXGI_ADAPTER_FLAG_SOFTWARE)&&(!chosen||desc.DedicatedVideoMemory>largest)){release(chosen);chosen=candidate;largest=desc.DedicatedVideoMemory;}else release(candidate);
  }
  release(selector);
 }
 HRESULT hr=D3D11CreateDevice(chosen,warp?D3D_DRIVER_TYPE_WARP:(chosen?D3D_DRIVER_TYPE_UNKNOWN:D3D_DRIVER_TYPE_HARDWARE),nullptr,0,levels,2,D3D11_SDK_VERSION,&device_,&level,&context_);release(chosen);
 if(FAILED(hr)&&!warp){release(context_);release(device_);hr=D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,levels,2,D3D11_SDK_VERSION,&device_,&level,&context_);Log::write("WARN","Hardware device failed; using WARP");}
 check(hr,"D3D11 device creation");featureLevel=level==D3D_FEATURE_LEVEL_11_0?"11.0":"10.1";
 IDXGIDevice* dxgi=nullptr;IDXGIAdapter* adapter=nullptr;IDXGIFactory1* factory=nullptr;
 check(device_->QueryInterface(__uuidof(IDXGIDevice),reinterpret_cast<void**>(&dxgi)),"DXGI device");hr=dxgi->GetAdapter(&adapter);release(dxgi);check(hr,"DXGI adapter");
 adapter->QueryInterface(__uuidof(IDXGIAdapter3),reinterpret_cast<void**>(&memoryAdapter_));
 DXGI_ADAPTER_DESC info{};if(SUCCEEDED(adapter->GetDesc(&info))){adapterLuid=info.AdapterLuid;gpu_=utf8(info.Description);videoMemory_=info.DedicatedVideoMemory;}
 hr=adapter->GetParent(__uuidof(IDXGIFactory1),reinterpret_cast<void**>(&factory));release(adapter);check(hr,"DXGI factory");
 IDXGIFactory2* factory2=nullptr;
 if(SUCCEEDED(factory->QueryInterface(__uuidof(IDXGIFactory2),reinterpret_cast<void**>(&factory2)))){
  DXGI_SWAP_CHAIN_DESC1 desc{};desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;desc.SampleDesc.Count=1;desc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;desc.BufferCount=2;desc.SwapEffect=DXGI_SWAP_EFFECT_FLIP_DISCARD;desc.Flags=DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
  IDXGISwapChain1* chain=nullptr;hr=factory2->CreateSwapChainForHwnd(device_,hwnd,&desc,nullptr,nullptr,&chain);release(factory2);
  if(SUCCEEDED(hr)){swap_=chain;flip_=true;IDXGISwapChain2* queue=nullptr;if(SUCCEEDED(chain->QueryInterface(__uuidof(IDXGISwapChain2),reinterpret_cast<void**>(&queue)))){queue->SetMaximumFrameLatency(1);latency_=queue->GetFrameLatencyWaitableObject();release(queue);}}else release(chain);
 }
 if(!swap_){DXGI_SWAP_CHAIN_DESC desc{};desc.BufferCount=2;desc.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;desc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;desc.OutputWindow=hwnd;desc.SampleDesc.Count=1;desc.Windowed=TRUE;desc.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;hr=factory->CreateSwapChain(device_,&desc,&swap_);}
 factory->MakeWindowAssociation(hwnd,DXGI_MWA_NO_ALT_ENTER);release(factory);check(hr,"Swap-chain creation");target();
 for(auto& q:queries_){D3D11_QUERY_DESC d{D3D11_QUERY_TIMESTAMP_DISJOINT,0};if(FAILED(device_->CreateQuery(&d,&q.disjoint)))continue;d.Query=D3D11_QUERY_TIMESTAMP;if(FAILED(device_->CreateQuery(&d,&q.begin))||FAILED(device_->CreateQuery(&d,&q.end))){release(q.disjoint);release(q.begin);release(q.end);}}
 Log::write("INFO","Renderer: "+gpu_+" / "+(flip_?"flip-discard":"compatibility discard"));
}
void Renderer::updateMemory(double now){
 if(lastMemory_>=0&&now-lastMemory_<.5)return;lastMemory_=now;memoryValid=false;
 if(memoryAdapter_){DXGI_QUERY_VIDEO_MEMORY_INFO info{};if(SUCCEEDED(memoryAdapter_->QueryVideoMemoryInfo(0,DXGI_MEMORY_SEGMENT_GROUP_LOCAL,&info))){memoryValid=true;processVram=info.CurrentUsage;vramBudget=info.Budget;}}
}
void Renderer::target(){ID3D11Texture2D* buffer=nullptr;check(swap_->GetBuffer(0,__uuidof(ID3D11Texture2D),reinterpret_cast<void**>(&buffer)),"Swap-chain buffer");HRESULT hr=device_->CreateRenderTargetView(buffer,nullptr,&target_);release(buffer);check(hr,"Render target");}
void Renderer::resize(unsigned w,unsigned h){if(!swap_||!w||!h)return;context_->OMSetRenderTargets(0,nullptr,nullptr);release(target_);check(swap_->ResizeBuffers(0,w,h,DXGI_FORMAT_UNKNOWN,flip_?DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT:0),"ResizeBuffers");target();}
void Renderer::waitForFrame(){if(latency_)WaitForSingleObjectEx(latency_,100,FALSE);}
void Renderer::collect(){
 for(auto& q:queries_)if(q.pending){D3D11_QUERY_DATA_TIMESTAMP_DISJOINT d{};UINT64 a=0,b=0;if(context_->GetData(q.disjoint,&d,sizeof(d),D3D11_ASYNC_GETDATA_DONOTFLUSH)!=S_OK)continue;
  if(d.Disjoint){q.pending=false;continue;}
  if(context_->GetData(q.begin,&a,sizeof(a),D3D11_ASYNC_GETDATA_DONOTFLUSH)==S_OK&&context_->GetData(q.end,&b,sizeof(b),D3D11_ASYNC_GETDATA_DONOTFLUSH)==S_OK){if(d.Frequency&&b>=a){gpuMs_=double(b-a)*1000./double(d.Frequency);gpuValid_=true;}q.pending=false;}
 }
}
void Renderer::clear(){collect();activeQuery_=-1;for(size_t i=0;i<queries_.size();++i){auto& q=queries_[i];if(q.disjoint&&!q.pending){context_->Begin(q.disjoint);context_->End(q.begin);activeQuery_=static_cast<int>(i);break;}}
 const float color[]={.035f,.04f,.055f,1};context_->OMSetRenderTargets(1,&target_,nullptr);context_->ClearRenderTargetView(target_,color);
}
bool Renderer::present(bool vsync){if(activeQuery_>=0){auto& q=queries_[static_cast<size_t>(activeQuery_)];context_->End(q.end);context_->End(q.disjoint);q.pending=true;activeQuery_=-1;}
 HRESULT hr=swap_->Present(vsync?1:0,0);if(hr==DXGI_STATUS_OCCLUDED)return false;check(hr,"Present / device lost");return true;}
bool Renderer::visible(){HRESULT hr=swap_->Present(0,DXGI_PRESENT_TEST);if(hr==DXGI_STATUS_OCCLUDED)return false;check(hr,"Visibility test");return true;}
}
