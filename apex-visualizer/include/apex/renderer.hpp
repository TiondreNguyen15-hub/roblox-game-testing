#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <string>
namespace apex {
class Renderer {
public:
 Renderer()=default;~Renderer();Renderer(const Renderer&)=delete;Renderer& operator=(const Renderer&)=delete;
 void initialize(HWND window,bool warp);void resize(unsigned width,unsigned height);void clear();bool present(bool vsync);bool visible();
 ID3D11Device* device()const{return device_;}ID3D11DeviceContext* context()const{return context_;}
 const std::string& gpu()const{return gpu_;}unsigned long long videoMemory()const{return videoMemory_;}
private:
 void target();void cleanup()noexcept;
 ID3D11Device* device_=nullptr;ID3D11DeviceContext* context_=nullptr;IDXGISwapChain* swap_=nullptr;ID3D11RenderTargetView* target_=nullptr;
 std::string gpu_;unsigned long long videoMemory_=0;
};
}
