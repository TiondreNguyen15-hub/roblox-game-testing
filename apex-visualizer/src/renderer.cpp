#include "apex/renderer.hpp"
#include <stdexcept>
#include <sstream>
namespace apex {
namespace {
template<class T>void release(T*& p){if(p){p->Release();p=nullptr;}}
void check(HRESULT hr,const char* stage){if(FAILED(hr)){std::ostringstream s;s<<stage<<" failed (HRESULT 0x"<<std::hex<<static_cast<unsigned long>(hr)<<")";throw std::runtime_error(s.str());}}
std::string utf8(const wchar_t* p){int n=WideCharToMultiByte(CP_UTF8,0,p,-1,nullptr,0,nullptr,nullptr);if(n<=1)return "Unknown adapter";std::string s(static_cast<size_t>(n),'\0');WideCharToMultiByte(CP_UTF8,0,p,-1,s.data(),n,nullptr,nullptr);s.pop_back();return s;}
}
Renderer::~Renderer(){cleanup();}
void Renderer::cleanup()noexcept{if(context_)context_->ClearState();release(target_);release(swap_);release(context_);release(device_);}
void Renderer::initialize(HWND hwnd,bool warp){
 DXGI_SWAP_CHAIN_DESC desc{};desc.BufferCount=2;desc.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;desc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;desc.OutputWindow=hwnd;desc.SampleDesc.Count=1;desc.Windowed=TRUE;desc.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;
 const D3D_FEATURE_LEVEL levels[]={D3D_FEATURE_LEVEL_11_0,D3D_FEATURE_LEVEL_10_1};D3D_FEATURE_LEVEL level{};
 HRESULT hr=D3D11CreateDeviceAndSwapChain(nullptr,warp?D3D_DRIVER_TYPE_WARP:D3D_DRIVER_TYPE_HARDWARE,nullptr,0,levels,2,D3D11_SDK_VERSION,&desc,&swap_,&device_,&level,&context_);
 if(FAILED(hr)&&!warp){cleanup();hr=D3D11CreateDeviceAndSwapChain(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,levels,2,D3D11_SDK_VERSION,&desc,&swap_,&device_,&level,&context_);}
 check(hr,"D3D11 device creation");target();
 IDXGIDevice* dxgi=nullptr;IDXGIAdapter* adapter=nullptr;
 if(SUCCEEDED(device_->QueryInterface(__uuidof(IDXGIDevice),reinterpret_cast<void**>(&dxgi)))){
  if(SUCCEEDED(dxgi->GetAdapter(&adapter))){DXGI_ADAPTER_DESC info{};if(SUCCEEDED(adapter->GetDesc(&info))){gpu_=utf8(info.Description);videoMemory_=info.DedicatedVideoMemory;}release(adapter);}release(dxgi);
 }
 if(gpu_.empty())gpu_="Adapter information unavailable";
}
void Renderer::target(){ID3D11Texture2D* buffer=nullptr;check(swap_->GetBuffer(0,__uuidof(ID3D11Texture2D),reinterpret_cast<void**>(&buffer)),"Swap-chain buffer");HRESULT hr=device_->CreateRenderTargetView(buffer,nullptr,&target_);release(buffer);check(hr,"Render target");}
void Renderer::resize(unsigned w,unsigned h){if(!swap_||!w||!h)return;context_->OMSetRenderTargets(0,nullptr,nullptr);release(target_);check(swap_->ResizeBuffers(0,w,h,DXGI_FORMAT_UNKNOWN,0),"ResizeBuffers");target();}
void Renderer::clear(){const float color[]={.035f,.04f,.055f,1};context_->OMSetRenderTargets(1,&target_,nullptr);context_->ClearRenderTargetView(target_,color);}
bool Renderer::present(bool vsync){HRESULT hr=swap_->Present(vsync?1:0,0);if(hr==DXGI_STATUS_OCCLUDED)return false;check(hr,"Present / device lost");return true;}
bool Renderer::visible(){HRESULT hr=swap_->Present(0,DXGI_PRESENT_TEST);if(hr==DXGI_STATUS_OCCLUDED)return false;check(hr,"Visibility test");return true;}
}
