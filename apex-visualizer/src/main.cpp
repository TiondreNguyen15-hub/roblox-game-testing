#include <algorithm>
#include "apex/renderer.hpp"
#include "apex/menu.hpp"
#include "apex/visuals.hpp"
#include "apex/config.hpp"
#include "apex/telemetry.hpp"
#include "apex/provider.hpp"
#include "apex/log.hpp"
#include "apex/hardware.hpp"
#include "apex/frame_pacer.hpp"
#include "apex/benchmark.hpp"
#include "apex/compute.hpp"
#include <cmath>
#include <cstring>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <shellapi.h>
#include <commdlg.h>
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
std::filesystem::path chooseFile(HWND owner,bool save){
 wchar_t path[32768]{};if(save)std::copy_n(L"hud-readings.csv",16,path);OPENFILENAMEW info{};info.lStructSize=sizeof(info);info.hwndOwner=owner;info.lpstrFile=path;info.nMaxFile=32768;info.lpstrFilter=save?L"CSV files\0*.csv\0\0":L"Video recordings\0*.mp4;*.m4v;*.wmv;*.avi\0All files\0*.*\0\0";info.lpstrTitle=save?L"Export actual HUD OCR observations":L"Open your local gameplay recording";info.lpstrDefExt=save?L"csv":nullptr;info.Flags=OFN_EXPLORER|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST|(save?OFN_OVERWRITEPROMPT:OFN_FILEMUSTEXIST);if(save?GetSaveFileNameW(&info):GetOpenFileNameW(&info))return path;auto error=CommDlgExtendedError();if(error)throw std::runtime_error("File picker failed: "+std::to_string(error));return {};
}
struct CaptureTexture {
 ID3D11Texture2D* texture=nullptr;ID3D11ShaderResourceView* view=nullptr;unsigned width=0,height=0;std::uint64_t sequence=0;
 ~CaptureTexture(){clear();}void clear(){if(view)view->Release();if(texture)texture->Release();view=nullptr;texture=nullptr;sequence=0;width=height=0;}
 void update(apex::Renderer& r,const std::shared_ptr<const apex::CapturedFrame>& f){if(!f||sequence==f->sequence)return;
  if(width!=f->width||height!=f->height){clear();D3D11_TEXTURE2D_DESC d{};d.Width=f->width;d.Height=f->height;d.MipLevels=1;d.ArraySize=1;d.Format=DXGI_FORMAT_R8G8B8A8_UNORM;d.SampleDesc.Count=1;d.Usage=D3D11_USAGE_DYNAMIC;d.BindFlags=D3D11_BIND_SHADER_RESOURCE;d.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
   if(FAILED(r.device()->CreateTexture2D(&d,nullptr,&texture))||FAILED(r.device()->CreateShaderResourceView(texture,nullptr,&view))){clear();throw std::runtime_error("Capture preview texture allocation failed");}width=f->width;height=f->height;}
  D3D11_MAPPED_SUBRESOURCE m{};if(FAILED(r.context()->Map(texture,0,D3D11_MAP_WRITE_DISCARD,0,&m)))throw std::runtime_error("Capture preview texture map failed");for(unsigned y=0;y<height;++y)std::memcpy(static_cast<unsigned char*>(m.pData)+size_t(y)*m.RowPitch,f->rgba.data()+size_t(y)*width*4,size_t(width)*4);r.context()->Unmap(texture,0);sequence=f->sequence;
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
 bool captureCameraTest=false;bool captureTest=false;std::filesystem::path captureFile;bool capturePreviewDrawn=false;bool modelBenchmark=false,gpuSoak=false;double cpuSeconds=10,gpuSeconds=10;std::filesystem::path modelPath,inputProfile;bool computeCli=false,requireCuda=false;double computeSeconds=0,computeCancelAfter=-1;int computeIterations=200;bool smoke=false,warp=false,asyncSmoke=false;int maxFrames=0;double benchmarkSeconds=0,warmup=5;int cap=144,entities=64;bool benchmarkRequested=false,noVsync=false,simulate=false;std::filesystem::path jsonPath,benchmarkPath=L"benchmark-results";
 int argc=0;LPWSTR* argv=CommandLineToArgvW(GetCommandLineW(),&argc);
 if(argv){for(int i=1;i<argc;++i){std::wstring a=argv[i];if(a==L"--smoke-test"||a==L"--async-smoke-test"){smoke=true;warp=true;asyncSmoke=a==L"--async-smoke-test";maxFrames=asyncSmoke?60:6;}else if(a==L"--json"&&i+1<argc)jsonPath=argv[++i];else if(a==L"--warp")warp=true;
 else if(a==L"--capture-camera-test"){captureCameraTest=true;captureTest=true;smoke=true;warp=true;maxFrames=2000;}
 else if(a==L"--capture-file"&&i+1<argc)captureFile=argv[++i];else if(a==L"--capture-test"&&i+1<argc){captureTest=true;smoke=true;warp=true;maxFrames=2000;captureFile=argv[++i];}
 else if(a==L"--model-benchmark"){computeCli=true;modelBenchmark=true;}else if(a==L"--gpu-soak"){computeCli=true;modelBenchmark=true;gpuSoak=true;requireCuda=true;}
 else if(a==L"--model"&&i+1<argc)modelPath=argv[++i];else if(a==L"--input-profile"&&i+1<argc)inputProfile=argv[++i];
 else if((a==L"--cpu-seconds"||a==L"--gpu-seconds")&&i+1<argc){try{size_t used=0;std::wstring raw=argv[++i];double v=std::stod(raw,&used);if(used!=raw.size()||!std::isfinite(v)||v<1||v>86400)throw std::runtime_error("Invalid model benchmark duration");if(a==L"--cpu-seconds")cpuSeconds=v;else gpuSeconds=v;}catch(...){LocalFree(argv);return 2;}}
 else if(a==L"--compute-test")computeCli=true;else if(a==L"--require-cuda")requireCuda=true;
 else if((a==L"--compute-seconds"||a==L"--compute-iterations"||a==L"--compute-cancel-after")&&i+1<argc){try{double v=std::stod(argv[++i]);if(!std::isfinite(v)||v<0||v>86400)throw std::runtime_error("Invalid compute limit");if(a==L"--compute-cancel-after")computeCancelAfter=v;else if(a==L"--compute-seconds")computeSeconds=v;else {if(v<1||v!=std::floor(v))throw std::runtime_error("Invalid iterations");computeIterations=static_cast<int>(v);}}catch(...){LocalFree(argv);return 2;}}
 else if(a==L"--simulate")simulate=true;else if(a==L"--no-vsync")noVsync=true;
 else if(a==L"--output"&&i+1<argc)benchmarkPath=argv[++i];
 else if((a==L"--benchmark"||a==L"--warmup"||a==L"--cap"||a==L"--entities")&&i+1<argc){try{double v=std::stod(argv[++i]);if(!std::isfinite(v))throw std::runtime_error("Nonfinite argument");if(a==L"--benchmark"){benchmarkRequested=true;benchmarkSeconds=v;}else if(a==L"--warmup")warmup=v;else if(a==L"--cap"){if(v<30||v>360)throw std::runtime_error("Cap range 30..360");cap=static_cast<int>(v);}else{if(v<1||v>64)throw std::runtime_error("Entity range 1..64");entities=static_cast<int>(v);}}catch(...){LocalFree(argv);return 2;}}
else if(a==L"--frames"&&i+1<argc){try{maxFrames=std::max(1,std::stoi(argv[++i]));}catch(...){LocalFree(argv);return 2;}}}LocalFree(argv);}
 if(gpuSoak)gpuSeconds=std::max(300.,gpuSeconds);
 apex::Benchmark benchmark;bool benchmarkStarted=false;
 try{
  apex::Log::write("INFO","Application startup");
  ImGui_ImplWin32_EnableDpiAwareness();Window window;window.open();apex::Renderer renderer;renderer.initialize(window.handle,warp);Gui gui;gui.initialize(window.handle,renderer);
  apex::Settings settings;std::error_code pathError;std::filesystem::path exeFolder;
  wchar_t modulePath[32768];DWORD moduleLength=GetModuleFileNameW(nullptr,modulePath,32768);if(moduleLength&&moduleLength<32768)exeFolder=std::filesystem::path(modulePath).parent_path();std::string message;if(!benchmarkRequested&&!computeCli&&std::filesystem::exists(apex::Config::defaultPath())&&!apex::Config::load(apex::Config::defaultPath(),settings,message)&&!smoke)MessageBoxA(window.handle,message.c_str(),"Configuration warning",MB_OK|MB_ICONWARNING);
  if(computeCli&&benchmarkRequested)throw std::runtime_error("Run compute and rendering benchmarks separately");if(computeCli){settings.liveFile=false;settings.entityCount=64;settings.vsync=false;settings.frameLimit=144;}
  if(benchmarkRequested){settings.entityCount=entities;settings.frameLimit=cap;settings.vsync=!noVsync;settings.liveFile=!jsonPath.empty();}if(simulate)settings.liveFile=false;
  if(!jsonPath.empty()){auto value=jsonPath.u8string();if(value.size()>200)throw std::runtime_error("JSON path exceeds 200 UTF-8 bytes");std::copy(value.begin(),value.end(),settings.livePath);settings.livePath[value.size()]=0;settings.liveFile=true;}
  if(jsonPath.empty()&&std::string(settings.livePath)=="apex_entities.json"&&!std::filesystem::exists(std::filesystem::u8path(settings.livePath),pathError)){auto sample=exeFolder/L"apex_entities.json";auto value=sample.u8string();if(value.size()<sizeof(settings.livePath)&&std::filesystem::exists(sample,pathError)){std::copy(value.begin(),value.end(),settings.livePath);settings.livePath[value.size()]=0;}}
  apex::DeviceInfo deviceInfo{renderer.gpu(),renderer.videoMemory()};
  apex::HardwareMetrics hardware;hardware.initialize(renderer.gpu(),renderer.adapterLuid);deviceInfo.cudaStatus=hardware.cudaStatus;deviceInfo.nvmlStatus=hardware.nvmlStatus;deviceInfo.featureLevel=renderer.featureLevel;
  if(benchmarkRequested){benchmark.start(benchmarkPath,renderer.gpu(),"DirectX / feature level "+renderer.featureLevel+" / "+hardware.cudaStatus,benchmarkSeconds,warmup);benchmarkStarted=true;benchmark.metadata={{"vsync",settings.vsync?"on":"off"},{"frame_cap",std::to_string(settings.frameLimit)},{"entity_count",std::to_string(settings.entityCount)},{"source",settings.liveFile?settings.livePath:"local simulation"},{"window_initial_size","1440x900 outer; DPI dependent client"},{"nvml",hardware.nvmlStatus},{"cuda",hardware.cudaStatus},{"cuda_driver_api_version",std::to_string(hardware.cudaDriver)},{"cuda_compute_capability",std::to_string(hardware.computeMajor)+"."+std::to_string(hardware.computeMinor)},{"present",renderer.flipModel()?"flip-discard latency1":"compatibility discard"},{"model_backend","optional ONNX compute validation; inactive during render-only benchmark"}};}
  CaptureTexture captureTexture;apex::CaptureSource capture;apex::HudOcrService hud;
  apex::ComputeRunner compute;bool computeStarted=false,computeReported=false;double computeFinishedAt=-1,computeStartedAt=0;std::filesystem::path computeDirectory;
  apex::ProviderSystem providers(!smoke||asyncSmoke);apex::Visuals visuals;apex::Menu menu;apex::SystemMetrics metrics;apex::RenderStats stats;
  {std::string error;auto path=apex::Config::defaultPath().parent_path()/"hud-settings.json";if(std::filesystem::exists(path)&&!apex::loadHudOptions(path,menu.hudOptions,error))apex::Log::write("WARN",error);}
  if(captureTest){settings.menuWidth=1000;settings.menuHeight=780;}
  if(captureCameraTest||!captureFile.empty()){capture.start(captureFile);menu.selectPage(apex::MenuPage::Capture);}
  apex::FramePacer pacer;using Clock=std::chrono::steady_clock;auto previous=Clock::now();auto benchmarkStart=previous;double simulationTime=0;int frames=0;bool occluded=false;
  while(!state.quit){
   MSG msg{};while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){if(msg.message==WM_QUIT)state.quit=true;TranslateMessage(&msg);DispatchMessageW(&msg);}if(state.quit)break;
   if(state.minimized||(occluded&&!renderer.visible())){if(benchmarkRequested||computeCli)throw std::runtime_error("Benchmark interrupted: window minimized or occluded");std::this_thread::sleep_for(std::chrono::milliseconds(50));previous=Clock::now();continue;}
   occluded=false;if(state.width&&state.height){renderer.resize(state.width,state.height);state.width=state.height=0;}
   renderer.waitForFrame();
   auto start=Clock::now();float delta=std::chrono::duration<float>(start-previous).count();stats.wallFrameMs=std::chrono::duration<double,std::milli>(start-previous).count();previous=start;
   delta=std::min(delta,.1f);if(state.pause){settings.animate=!settings.animate;state.pause=false;}
   if(state.reset||menu.consumeReset()){simulationTime=0;state.reset=false;}
   if(settings.animate)simulationTime+=delta*settings.playbackSpeed;metrics.update();providers.update(settings,metrics.uptime(),simulationTime);stats.processingMs=providers.processingMs;stats.deliveryMs=providers.deliveryMs;
   hardware.update(metrics.uptime());renderer.updateMemory(metrics.uptime());deviceInfo.utilizationValid=hardware.utilizationValid;deviceInfo.utilization=hardware.utilization;deviceInfo.memoryValid=hardware.memoryValid;deviceInfo.usedBytes=hardware.usedBytes;deviceInfo.totalBytes=hardware.totalBytes;deviceInfo.processMemoryValid=renderer.memoryValid;deviceInfo.processVram=renderer.processVram;deviceInfo.vramBudget=renderer.vramBudget;deviceInfo.processUtilizationValid=hardware.processUtilizationValid;deviceInfo.processUtilization=hardware.processUtilization;deviceInfo.processCounterStatus=hardware.processCounterStatus;
   apex::ProcessObservation observation;observation.engineBusy=hardware.processUtilizationValid?hardware.processUtilization:-1;observation.localVramMiB=renderer.memoryValid?renderer.processVram/1048576.:-1;observation.workingMiB=metrics.memoryValid?metrics.workingSet/1048576.:-1;observation.privateMiB=metrics.memoryValid?metrics.privateBytes/1048576.:-1;observation.deviceBusy=hardware.utilizationValid?hardware.utilization:-1;observation.deviceVramMiB=hardware.memoryValid?hardware.usedBytes/1048576.:-1;compute.observe(stats.wallFrameMs,observation);
   int computeAction=menu.consumeCompute();if(computeAction==3)compute.cancel();
   if((computeCli&&!computeStarted&&metrics.uptime()>=1.1)||computeAction==1||computeAction==2||computeAction==4||computeAction==5){apex::ComputeOptions options;options.adapter=renderer.gpu();options.adapterLuid=(std::uint64_t(static_cast<unsigned long>(renderer.adapterLuid.HighPart))<<32)|renderer.adapterLuid.LowPart;options.baseDirectory=exeFolder;options.iterations=computeCli?computeIterations:200;options.phaseSeconds=computeCli?computeSeconds:computeAction==2?300:0;
    options.modelBenchmark=computeCli?modelBenchmark:(computeAction==4||computeAction==5);if(options.modelBenchmark){options.phaseSeconds=0;options.cpuSeconds=computeCli?cpuSeconds:10;options.gpuSeconds=computeCli?gpuSeconds:computeAction==5?300:10;options.modelPath=computeCli?apex::normalizeComputePath(modelPath,std::filesystem::current_path()):std::filesystem::u8path(menu.modelPath());options.inputProfile=computeCli?apex::normalizeComputePath(inputProfile,std::filesystem::current_path()):std::filesystem::u8path(menu.inputProfile());}
    computeDirectory=computeCli?benchmarkPath:apex::Config::defaultPath().parent_path()/L"compute-results"/std::to_wstring(std::chrono::system_clock::now().time_since_epoch().count());options.resultDirectory=computeDirectory;compute.start(options);computeStartedAt=metrics.uptime();computeStarted=true;computeReported=false;computeFinishedAt=-1;deviceInfo.computeReport.clear();}
   if(computeCli&&computeStarted&&computeCancelAfter>=0&&metrics.uptime()-computeStartedAt>=computeCancelAfter)compute.cancel();
   deviceInfo.compute=compute.snapshot();if(computeStarted&&deviceInfo.compute.finished&&!computeReported){if(computeFinishedAt<0)computeFinishedAt=metrics.uptime();if(metrics.uptime()-computeFinishedAt>=1.1){try{apex::saveComputeReport(computeDirectory,deviceInfo.compute);deviceInfo.computeReport=(computeDirectory/L"compute-report.json").u8string();}catch(const std::exception& reportError){apex::Log::write("ERROR",reportError.what());if(computeCli)throw;deviceInfo.computeReport=std::string("Report failed: ")+reportError.what();}compute.finishCapture();computeReported=true;apex::Log::write("INFO","Compute report status: "+deviceInfo.computeReport);if(computeCli)break;}}
   int captureAction=menu.consumeCapture();if(captureAction){try{if(captureAction==5)capture.setPaused(!capture.snapshot().paused);else if(captureAction==3)capture.stop();else {std::filesystem::path file;if(captureAction==4){file=chooseFile(window.handle,false);if(file.empty())captureAction=0;else menu.setRecordingPath(file.u8string());}else if(captureAction!=1)file=apex::normalizeComputePath(std::filesystem::u8path(menu.recordingPath()),exeFolder);if(captureAction){captureTexture.clear();hud.beginSource();capture.start(file);}}}catch(const std::exception& e){apex::Log::write("WARN",e.what());MessageBoxA(window.handle,e.what(),"Capture",MB_OK|MB_ICONWARNING);}}

   deviceInfo.capture=capture.snapshot();try{if(deviceInfo.capture.running||deviceInfo.capture.frame)captureTexture.update(renderer,deviceInfo.capture.frame);}catch(const std::exception& e){capture.stop();captureTexture.clear();apex::Log::write("WARN",e.what());deviceInfo.capture=capture.snapshot();deviceInfo.capture.message=e.what();}deviceInfo.captureTexture=reinterpret_cast<std::uintptr_t>(captureTexture.view);
   int hudAction=menu.consumeHud();try{if(hudAction==2){auto file=chooseFile(window.handle,true);if(!file.empty())hud.exportCsv(file);}else if((menu.hudEnabled||hudAction==1)&&deviceInfo.capture.isRecording&&deviceInfo.capture.frame)hud.submit(deviceInfo.capture.frame,menu.hudOptions,deviceInfo.capture.source,hudAction==1);}catch(const std::exception& e){apex::Log::write("WARN",e.what());MessageBoxA(window.handle,e.what(),"HUD OCR",MB_OK|MB_ICONWARNING);}deviceInfo.hud=hud.snapshot();
   ImGui_ImplDX11_NewFrame();ImGui_ImplWin32_NewFrame();ImGui::GetIO().FontGlobalScale=state.dpi;ImGui::NewFrame();
   deviceInfo.flipModel=renderer.flipModel();deviceInfo.gpuTimingValid=renderer.gpuTimingValid();deviceInfo.gpuMs=renderer.gpuTimeMs();
   auto size=ImGui::GetIO().DisplaySize;if(benchmarkRequested&&frames==0){benchmark.metadata["initial_client_width"]=std::to_string(int(size.x));benchmark.metadata["initial_client_height"]=std::to_string(int(size.y));}
   auto visualStats=visuals.draw(providers.entities(),settings,size.x,size.y);menu.draw(settings,deviceInfo,metrics,stats,visualStats,state.menu,delta,providers.status(metrics.uptime()));
   if(settings.liveFile&&!state.menu)ImGui::GetForegroundDrawList()->AddText({20,42},IM_COL32(210,190,255,255),providers.status(metrics.uptime()).c_str());
   ImGui::Render();auto* data=ImGui::GetDrawData();renderer.clear();ImGui_ImplDX11_RenderDrawData(data);
   stats.vertices=data->TotalVtxCount;stats.indices=data->TotalIdxCount;stats.drawCalls=0;
   for(int n=0;n<data->CmdListsCount;++n)for(const auto& cmd:data->CmdLists[n]->CmdBuffer)if(cmd.ElemCount&&!cmd.UserCallback)++stats.drawCalls;
   if(deviceInfo.captureTexture)for(int n=0;n<data->CmdListsCount;++n)for(const auto& cmd:data->CmdLists[n]->CmdBuffer)if(cmd.ElemCount&&!cmd.UserCallback&&cmd.GetTexID()==static_cast<ImTextureID>(deviceInfo.captureTexture))capturePreviewDrawn=true;
   stats.frameMs=std::chrono::duration<double,std::milli>(Clock::now()-start).count();occluded=!renderer.present(settings.vsync);
   ++frames;
   if(benchmarkRequested&&frames>1){apex::BenchmarkSample sample;sample.frameMs=stats.wallFrameMs;sample.cpuWorkMs=stats.frameMs;sample.processCpu=metrics.cpuValid?metrics.cpu():-1;sample.gpuMs=renderer.gpuTimingValid()?renderer.gpuTimeMs():-1;sample.gpuUtil=hardware.utilizationValid?double(hardware.utilization):-1;sample.deviceVramMiB=hardware.memoryValid?hardware.usedBytes/1048576.:-1;sample.processVramMiB=renderer.memoryValid?renderer.processVram/1048576.:-1;sample.workingSetMiB=metrics.memoryValid?metrics.workingSet/1048576.:-1;sample.privateMiB=metrics.memoryValid?metrics.privateBytes/1048576.:-1;sample.processingMs=stats.processingMs;sample.deliveryMs=stats.deliveryMs;
    if(benchmark.sample(std::chrono::duration<double>(Clock::now()-benchmarkStart).count(),sample))break;}
   if(captureCameraTest&&deviceInfo.capture.frames>=10&&capturePreviewDrawn)break;
   if(captureTest&&!deviceInfo.capture.running){if(deviceInfo.capture.state!="end_of_recording"||!deviceInfo.capture.frames||!captureTexture.sequence||!capturePreviewDrawn)throw std::runtime_error("External recording/preview integration failed: "+deviceInfo.capture.message);break;}
   if(maxFrames&&frames>=maxFrames)break;
   auto target=start+std::chrono::microseconds(1000000/settings.frameLimit);pacer.wait(target);
  }
  if(captureTest){if((captureCameraTest?deviceInfo.capture.frames<10:deviceInfo.capture.state!="end_of_recording")||!capturePreviewDrawn)throw std::runtime_error("External capture integration incomplete");std::ofstream report(captureCameraTest?"obs-camera-smoke-result.txt":"capture-smoke-result.txt");report<<(captureCameraTest?"PASS: actual OBS virtual-camera frames (no OBS producer; placeholder output) =":"PASS: real recording frames=")<<deviceInfo.capture.frames<<"; size="<<deviceInfo.capture.width<<"x"<<deviceInfo.capture.height<<"; last uploaded sequence="<<captureTexture.sequence<<"; actual DX11 preview draw command="<<capturePreviewDrawn<<"; WARP="<<warp<<'\n';report.flush();if(!report)throw std::runtime_error("Capture integration report write failed");}
  if(computeStarted&&!computeReported){compute.stop();compute.finishCapture();apex::saveComputeReport(computeDirectory,compute.snapshot());}
  if(computeCli){auto result=compute.snapshot();if(!computeReported||!apex::computeRunSucceeded(result,requireCuda))return 6;}
  if(benchmarkRequested)benchmark.finish("stopped before requested duration");
  if(smoke){std::ofstream report("smoke-result.txt");report<<"PASS: "<<frames<<" DX11/ImGui frames; GPU="<<renderer.gpu()<<"; vertices="<<stats.vertices<<"; drawCalls="<<stats.drawCalls<<"; present="<<(renderer.flipModel()?"flip":"discard")<<"; gpuTiming="<<(renderer.gpuTimingValid()?renderer.gpuTimeMs():-1)<<"; fileEntities="<<providers.entities().size()<<'\n';if(!jsonPath.empty()&&providers.entities().empty())return 4;if(frames<6||stats.vertices==0||stats.drawCalls==0)return 3;}
  apex::Log::write("INFO","Application shutdown");return 0;
 }catch(const std::exception& e){if(benchmarkStarted){try{benchmark.finish(std::string("failed: ")+e.what());}catch(const std::exception& reportError){apex::Log::write("ERROR",reportError.what());}}apex::Log::write("ERROR",e.what());if(!smoke&&!benchmarkRequested&&!computeCli)MessageBoxA(nullptr,e.what(),"Apex Visualization error",MB_OK|MB_ICONERROR);return 1;}
}
