#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <string>
#include <array>
namespace apex {
class Renderer {
public:
 Renderer()=default;~Renderer();Renderer(const Renderer&)=delete;Renderer& operator=(const Renderer&)=delete;
 void waitForFrame();bool flipModel()const{return flip_;}bool gpuTimingValid()const{return gpuValid_;}double gpuTimeMs()const{return gpuMs_;}
 void initialize(HWND window,bool warp);void resize(unsigned width,unsigned height);void clear();bool present(bool vsync);bool visible();
 ID3D11Device* device()const{return device_;}ID3D11DeviceContext* context()const{return context_;}
 const std::string& gpu()const{return gpu_;}unsigned long long videoMemory()const{return videoMemory_;}
private:
 void collect();void target();void cleanup()noexcept;
 ID3D11Device* device_=nullptr;ID3D11DeviceContext* context_=nullptr;IDXGISwapChain* swap_=nullptr;ID3D11RenderTargetView* target_=nullptr;
 struct Query{ID3D11Query* disjoint=nullptr;ID3D11Query* begin=nullptr;ID3D11Query* end=nullptr;bool pending=false;};
 std::array<Query,4> queries_{};int activeQuery_=-1;HANDLE latency_=nullptr;bool flip_=false,gpuValid_=false;double gpuMs_=0;
 std::string gpu_;unsigned long long videoMemory_=0;
};
}
