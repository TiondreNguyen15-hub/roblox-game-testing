#include <algorithm>
#include "apex/renderer.hpp"
#include "apex/menu.hpp"
#include "apex/visuals.hpp"
#include "apex/config.hpp"
#include "apex/telemetry.hpp"
#include "apex/provider.hpp"
#include "apex/log.hpp"
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <shellapi.h>
#include <chrono>
#include <thread>
#include <fstream>
#include <stdexcept>
#include <string>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND,UINT,WPARAM,LPARAM);
namespace {
struct WindowState {unsigned width=0,height=0;bool minimized=false,menu=true,quit=false,pause=false,reset=false;float dpi=1;};
WindowState state;
LRESULT WINAPI procedure(HWND hwnd,UINT msg,WPARAM w,LPARAM l){
 if(msg==WM_KEYDOWN&&!(l&(1LL<<30))){if(w==VK_F5){state.pause=true;return 0;}if(w==VK_F6){state.reset=true;return 0;}if(w==VK_INSERT){state.menu=!state.menu;return 0;}if(w==VK_F10){state.quit=true;return 0;}}
 if(msg==WM_GETMINMAXINFO){auto* info=reinterpret_cast<MINMAXINFO*>(l);info->ptMinTrackSize={640,480};return 0;}
 if(msg==WM_SIZE){state.minimized=w==SIZE_MINIMIZED;state.width=LOWORD(l);state.height=HIWORD(l);return 0;}
 if(msg==WM_DPICHANGED){state.dpi=float(HIWORD(w))/96.f;auto* r=reinterpret_cast<RECT*>(l);SetWindowPos(hwnd,nullptr,r->left,r->top,r->right-r->left,r->bottom-r->top,SWP_NOZORDER|SWP_NOACTIVATE);return 0;}
 if(msg==WM_DESTROY){PostQuitMessage(0);return 0;}
 if(msg==WM_SYSCOMMAND&&(w&0xfff0)==SC_KEYMENU)return 0;
 if(ImGui::GetCurrentContext()&&ImGui_ImplWin32_WndProcHandler(hwnd,msg,w,l))return 1;
 return DefWindowProcW(hwnd,msg,w,l);
}
struct Window {
 HINSTANCE instance=GetModuleHandleW(nullptr);HWND handle=nullptr;bool registered=false;
 ~Window(){if(handle&&IsWindow(handle))DestroyWindow(handle);if(registered)UnregisterClassW(L"ApexVisualizationWindow",instance);}
 void open(){
  WNDCLASSEXW wc{};wc.cbSize=sizeof(wc);wc.style=CS_CLASSDC;wc.lpfnWndProc=procedure;wc.hInstance=instance;wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);wc.lpszClassName=L"ApexVisualizationWindow";
  if(!RegisterClassExW(&wc))throw std::runtime_error("Window class registration failed");registered=true;
  handle=CreateWindowExW(0,wc.lpszClassName,L"Apex Visualization - Simulated Entity Lab",WS_OVERLAPPEDWINDOW,100,70,1440,900,nullptr,nullptr,instance,nullptr);
  if(!handle)throw std::runtime_error("Window creation failed");using DpiFn=UINT(WINAPI*)(HWND);auto dpi=reinterpret_cast<DpiFn>(GetProcAddress(GetModuleHandleW(L"user32.dll"),"GetDpiForWindow"));if(dpi)state.dpi=float(dpi(handle))/96.f;ShowWindow(handle,SW_SHOWDEFAULT);UpdateWindow(handle);
 }
};
struct Gui {
 bool context=false,platform=false,graphics=false;
 ~Gui(){if(graphics)ImGui_ImplDX11_Shutdown();if(platform)ImGui_ImplWin32_Shutdown();if(context)ImGui::DestroyContext();}
 void initialize(HWND hwnd,apex::Renderer& renderer){
  IMGUI_CHECKVERSION();ImGui::CreateContext();context=true;ImGui::GetIO().IniFilename=nullptr;ImGui::GetIO().ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard;ImGui::StyleColorsDark();
  wchar_t windowsFolder[MAX_PATH];UINT length=GetWindowsDirectoryW(windowsFolder,MAX_PATH);
  if(length&&length<MAX_PATH){auto font=std::filesystem::path(windowsFolder)/L"Fonts"/L"segoeui.ttf";if(std::filesystem::exists(font))ImGui::GetIO().Fonts->AddFontFromFileTTF(font.u8string().c_str(),17.f);}

  if(!ImGui_ImplWin32_Init(hwnd))throw std::runtime_error("ImGui Win32 initialization failed");platform=true;
  if(!ImGui_ImplDX11_Init(renderer.device(),renderer.context()))throw std::runtime_error("ImGui DX11 initialization failed");graphics=true;if(!ImGui_ImplDX11_CreateDeviceObjects())throw std::runtime_error("ImGui DX11 shader/font resources could not be created");
 }
};
}
int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR,int){
 bool smoke=false,warp=false,asyncSmoke=false;int maxFrames=0;std::filesystem::path jsonPath;
 int argc=0;LPWSTR* argv=CommandLineToArgvW(GetCommandLineW(),&argc);
 if(argv){for(int i=1;i<argc;++i){std::wstring a=argv[i];if(a==L"--smoke-test"||a==L"--async-smoke-test"){smoke=true;warp=true;asyncSmoke=a==L"--async-smoke-test";maxFrames=asyncSmoke?60:6;}else if(a==L"--json"&&i+1<argc)jsonPath=argv[++i];else if(a==L"--warp")warp=true;else if(a==L"--frames"&&i+1<argc){try{maxFrames=std::max(1,std::stoi(argv[++i]));}catch(...){LocalFree(argv);return 2;}}}LocalFree(argv);}
 try{
  apex::Log::write("INFO","Application startup");
  ImGui_ImplWin32_EnableDpiAwareness();Window window;window.open();apex::Renderer renderer;renderer.initialize(window.handle,warp);Gui gui;gui.initialize(window.handle,renderer);
  apex::Settings settings;std::error_code pathError;std::filesystem::path exeFolder;
  wchar_t modulePath[32768];DWORD moduleLength=GetModuleFileNameW(nullptr,modulePath,32768);if(moduleLength&&moduleLength<32768)exeFolder=std::filesystem::path(modulePath).parent_path();std::string message;if(std::filesystem::exists(apex::Config::defaultPath())&&!apex::Config::load(apex::Config::defaultPath(),settings,message)&&!smoke)MessageBoxA(window.handle,message.c_str(),"Configuration warning",MB_OK|MB_ICONWARNING);
  if(!jsonPath.empty()){auto value=jsonPath.u8string();if(value.size()>200)throw std::runtime_error("JSON path exceeds 200 UTF-8 bytes");std::copy(value.begin(),value.end(),settings.livePath);settings.livePath[value.size()]=0;settings.liveFile=true;}
  if(jsonPath.empty()&&std::string(settings.livePath)=="apex_entities.json"&&!std::filesystem::exists(std::filesystem::u8path(settings.livePath),pathError)){auto sample=exeFolder/L"apex_entities.json";auto value=sample.u8string();if(value.size()<sizeof(settings.livePath)&&std::filesystem::exists(sample,pathError)){std::copy(value.begin(),value.end(),settings.livePath);settings.livePath[value.size()]=0;}}
  apex::DeviceInfo deviceInfo{renderer.gpu(),renderer.videoMemory()};
  apex::ProviderSystem providers(!smoke||asyncSmoke);apex::Visuals visuals;apex::Menu menu;apex::SystemMetrics metrics;apex::RenderStats stats;
  using Clock=std::chrono::steady_clock;auto previous=Clock::now();double simulationTime=0;int frames=0;bool occluded=false;
  while(!state.quit){
   MSG msg{};while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){if(msg.message==WM_QUIT)state.quit=true;TranslateMessage(&msg);DispatchMessageW(&msg);}if(state.quit)break;
   if(state.minimized||(occluded&&!renderer.visible())){std::this_thread::sleep_for(std::chrono::milliseconds(50));previous=Clock::now();continue;}
   occluded=false;if(state.width&&state.height){renderer.resize(state.width,state.height);state.width=state.height=0;}
   renderer.waitForFrame();
   auto start=Clock::now();float delta=std::chrono::duration<float>(start-previous).count();previous=start;
   delta=std::min(delta,.1f);if(state.pause){settings.animate=!settings.animate;state.pause=false;}
   if(state.reset||menu.consumeReset()){simulationTime=0;state.reset=false;}
   if(settings.animate)simulationTime+=delta*settings.playbackSpeed;metrics.update();providers.update(settings,metrics.uptime(),simulationTime);
   ImGui_ImplDX11_NewFrame();ImGui_ImplWin32_NewFrame();ImGui::GetIO().FontGlobalScale=state.dpi;ImGui::NewFrame();
   deviceInfo.flipModel=renderer.flipModel();deviceInfo.gpuTimingValid=renderer.gpuTimingValid();deviceInfo.gpuMs=renderer.gpuTimeMs();
   auto size=ImGui::GetIO().DisplaySize;auto visualStats=visuals.draw(providers.entities(),settings,size.x,size.y);menu.draw(settings,deviceInfo,metrics,stats,visualStats,state.menu,delta,providers.status(metrics.uptime()));
   if(settings.liveFile&&!state.menu)ImGui::GetForegroundDrawList()->AddText({20,42},IM_COL32(210,190,255,255),providers.status(metrics.uptime()).c_str());
   ImGui::Render();auto* data=ImGui::GetDrawData();renderer.clear();ImGui_ImplDX11_RenderDrawData(data);
   stats.vertices=data->TotalVtxCount;stats.indices=data->TotalIdxCount;stats.drawCalls=0;
   for(int n=0;n<data->CmdListsCount;++n)for(const auto& cmd:data->CmdLists[n]->CmdBuffer)if(cmd.ElemCount&&!cmd.UserCallback)++stats.drawCalls;
   stats.frameMs=std::chrono::duration<double,std::milli>(Clock::now()-start).count();occluded=!renderer.present(settings.vsync);
   ++frames;if(maxFrames&&frames>=maxFrames)break;
   auto target=start+std::chrono::microseconds(1000000/settings.frameLimit);std::this_thread::sleep_until(target);
  }
  if(smoke){std::ofstream report("smoke-result.txt");report<<"PASS: "<<frames<<" DX11/ImGui frames; GPU="<<renderer.gpu()<<"; vertices="<<stats.vertices<<"; drawCalls="<<stats.drawCalls<<"; present="<<(renderer.flipModel()?"flip":"discard")<<"; gpuTiming="<<(renderer.gpuTimingValid()?renderer.gpuTimeMs():-1)<<"; fileEntities="<<providers.entities().size()<<'\n';if(!jsonPath.empty()&&providers.entities().empty())return 4;if(frames<6||stats.vertices==0||stats.drawCalls==0)return 3;}
  apex::Log::write("INFO","Application shutdown");return 0;
 }catch(const std::exception& e){apex::Log::write("ERROR",e.what());if(!smoke)MessageBoxA(nullptr,e.what(),"Apex Visualization error",MB_OK|MB_ICONERROR);return 1;}
}
