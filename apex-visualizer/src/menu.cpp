#include "apex/menu.hpp"
#include "apex/config.hpp"
#include "apex/log.hpp"
#include <imgui.h>
#include <algorithm>
#include <cmath>
namespace apex {
void Menu::draw(Settings& s,const DeviceInfo& device,const SystemMetrics& system,const RenderStats& stats,const VisualStats& visuals,bool shown,float dt,const std::string& sourceStatus){
 fade_+=(float(shown)-fade_)*(1-std::exp(-dt*15));if(!shown&&fade_<.01f)return;
 ImGuiStyle& style=ImGui::GetStyle();style.WindowRounding=12;style.ChildRounding=8;style.FrameRounding=5;style.WindowBorderSize=1;style.FramePadding={9,6};style.ItemSpacing={10,10};
 style.Colors[ImGuiCol_WindowBg]={.055f,.055f,.078f,.98f};style.Colors[ImGuiCol_ChildBg]={.075f,.075f,.1f,1};style.Colors[ImGuiCol_Border]={.2f,.18f,.29f,1};
 style.Colors[ImGuiCol_Button]={.15f,.12f,.22f,1};style.Colors[ImGuiCol_ButtonHovered]={s.accent[0]*.7f,s.accent[1]*.7f,s.accent[2]*.7f,1};style.Colors[ImGuiCol_ButtonActive]={s.accent[0],s.accent[1],s.accent[2],1};style.Colors[ImGuiCol_CheckMark]={s.accent[0],s.accent[1],s.accent[2],1};style.Colors[ImGuiCol_SliderGrab]=style.Colors[ImGuiCol_CheckMark];style.Colors[ImGuiCol_Header]=style.Colors[ImGuiCol_Button];style.Colors[ImGuiCol_HeaderHovered]=style.Colors[ImGuiCol_ButtonHovered];
 auto screen=ImGui::GetIO().DisplaySize;float scale=ImGui::GetIO().FontGlobalScale;
 ImGui::SetNextWindowSize({std::min(s.menuWidth*scale,screen.x-24),std::min(s.menuHeight*scale,screen.y-24)},ImGuiCond_Always);
 ImGui::SetNextWindowPos({screen.x*.5f,screen.y*.5f},ImGuiCond_Always,{.5f,.5f});
 ImGui::PushStyleVar(ImGuiStyleVar_Alpha,fade_);
 ImGuiWindowFlags flags=ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoTitleBar;
 if(!shown)flags|=ImGuiWindowFlags_NoInputs;
 if(ImGui::Begin("Apex Visualization",nullptr,flags)){
  ImGui::TextColored({s.accent[0],s.accent[1],s.accent[2],1},"A /  APEX VISUALIZATION");ImGui::SameLine();ImGui::TextDisabled("  STANDALONE LAB");ImGui::Separator();
  ImGui::BeginChild("Sidebar",{135*scale,-28},true);
  const char* tabs[]={"Visuals","Rendering","System","Configuration","Capture"};for(int i=0;i<5;++i)if(ImGui::Selectable(tabs[i],tab_==i,0,{0,36*scale}))tab_=i;
  ImGui::Spacing();ImGui::TextDisabled("INSERT");ImGui::TextWrapped("Toggle menu\nF10: exit");ImGui::EndChild();ImGui::SameLine();
  ImGui::BeginChild("Page",{0,-28},false);
  ImGui::Text("%s",tabs[tab_]);ImGui::Separator();
  if(tab_==0){
   ImGui::Checkbox("Read local JSON file",&s.liveFile);
   bool apply=ImGui::InputText("JSON path",s.livePath,sizeof(s.livePath),ImGuiInputTextFlags_AutoSelectAll|ImGuiInputTextFlags_EnterReturnsTrue);
   if(apply||ImGui::IsItemDeactivatedAfterEdit()){std::string error;if(!applyJsonPath(s,s.livePath,error))notice_=error;}
   if(ImGui::BeginPopupContextItem("JsonPathActions")){
    if(ImGui::MenuItem("Copy path"))ImGui::SetClipboardText(s.livePath);
    if(ImGui::MenuItem("Paste path")){const char* clip=ImGui::GetClipboardText();std::string error;notice_=applyJsonPath(s,clip?clip:"",error)?"Path pasted.":error;}
    ImGui::EndPopup();
   }
   if(ImGui::Button("Copy path"))ImGui::SetClipboardText(s.livePath);ImGui::SameLine();
   if(ImGui::Button("Paste path")){const char* clip=ImGui::GetClipboardText();std::string error;notice_=applyJsonPath(s,clip?clip:"",error)?"Path pasted.":error;}
   ImGui::TextDisabled("Ctrl+A / Ctrl+C / Ctrl+V; quoted Windows paths are accepted.");
   if(!notice_.empty())ImGui::TextWrapped("%s",notice_.c_str());
   if(s.liveFile)ImGui::TextWrapped("%s",sourceStatus.c_str());
   ImGui::TextDisabled(s.liveFile?"Entities come only from your local JSON file.":"Entities are generated inside this application.");
   ImGui::Checkbox("2D bounding boxes",&s.boxes);ImGui::SameLine();ImGui::Checkbox("Corner style",&s.corners);
   ImGui::Checkbox("Skeleton",&s.skeleton);ImGui::SameLine();ImGui::Checkbox("Health bars",&s.health);ImGui::Checkbox("Shield bars (JSON)",&s.shields);
   ImGui::Checkbox("Names",&s.names);ImGui::SameLine();ImGui::Checkbox("Distances",&s.distance);
   ImGui::Checkbox("Team identifiers",&s.teams);ImGui::SameLine();ImGui::Checkbox("Visibility labels",&s.visibility);
   ImGui::Checkbox("Enemies only (hide team 1)",&s.enemiesOnly);ImGui::Checkbox("Visible only",&s.visibleOnly);
   ImGui::SliderFloat("Max distance",&s.maxDistance,10,300,"%.0f m");ImGui::SliderInt("Simulated entities",&s.entityCount,1,64);
  }else if(tab_==1){
   ImGui::Checkbox("VSync",&s.vsync);ImGui::Checkbox("Animate simulated scene",&s.animate);ImGui::SliderFloat("Playback speed",&s.playbackSpeed,.1f,3.f,"%.1fx");
   const char* scenes[]={"Arena patrol","Tracking lanes","Static inspection"};ImGui::Combo("Training scene",&s.scene,scenes,3);
   if(ImGui::Button("Reset simulation (F6)"))reset_=true;
   ImGui::TextDisabled("F5: pause/resume | F6: reset scene");
   ImGui::SliderInt("Frame cap",&s.frameLimit,30,360);
   ImGui::SliderFloat("Menu width",&s.menuWidth,560,1200,"%.0f px");ImGui::SliderFloat("Menu height",&s.menuHeight,440,900,"%.0f px");ImGui::ColorEdit3("Purple accent",s.accent);
   ImGui::TextWrapped("VSync uses the monitor refresh rate. The frame cap also limits CPU usage. The scene pauses while minimized or occluded.");
  }else if(tab_==2){
   ImGui::TextWrapped("Compute status: %s",device.compute.providerStatus.c_str());
   if(device.compute.running){ImGui::TextWrapped("Running: %s",device.compute.phase.c_str());if(ImGui::Button("Cancel compute test"))computeRequest_=3;}
   else {if(ImGui::Button("Run CUDA + ONNX validation"))computeRequest_=1;if(ImGui::Button("Sustained test (5 min per available phase)"))computeRequest_=2;}
   ImGui::InputText("Model path (blank = bundled)",modelPath_,sizeof(modelPath_));
   ImGui::InputText("Input shape JSON (optional)",inputProfile_,sizeof(inputProfile_));
   ImGui::TextWrapped("FP32 tensors; dynamic dimensions require a shape profile. Ctrl+V supported. Paths apply to model benchmarks only.");
   if(!device.compute.running){if(ImGui::Button("Benchmark loaded model CPU / GPU"))computeRequest_=4;if(ImGui::Button("5-minute GPU model test"))computeRequest_=5;}
   if(!device.compute.modelPath.empty())ImGui::TextWrapped("Model: %s\nSHA256: %s",device.compute.modelPath.c_str(),device.compute.modelSha256.c_str());
   for(const auto* r:{&device.compute.cpu,&device.compute.gpu})if(r->latency.count){ImGui::Text("%s: %.4f ms Run; %.1f inference FPS (%llu runs)",r==&device.compute.cpu?"CPU":"GPU",r->latency.mean(),r->measuredWallSeconds>0?r->latency.count/r->measuredWallSeconds:0.,static_cast<unsigned long long>(r->latency.count));}
   if(device.compute.modelBenchmark){const auto& r=device.compute.gpu;ImGui::Text("GPU measured %.1f / %.1f seconds; %s",std::max(0.,r.measuredWallSeconds),device.compute.gpuTargetSeconds,r.state.c_str());for(const auto* trend:{&device.compute.vramTrend,&device.compute.privateTrend}){ImGui::Text("%s: %s, growth %.1f MiB, %.2f MiB/min",trend==&device.compute.vramTrend?"VRAM trend":"Private RAM trend",trend->state.c_str(),trend->growthMiB,trend->slopeMiBPerMinute);ImGui::TextWrapped("%s",trend->message.c_str());}}
   if(device.compute.finished){const auto& c=device.compute;ImGui::Text("GPU execution: %s",c.phase=="completed"&&c.gpu.verified?"PASS":c.gpu.state=="failed"?"FAIL":"INCOMPLETE");if(c.modelBenchmark&&c.gpuTargetSeconds>=300)ImGui::Text("Five-minute GPU execution: %s",c.phase=="completed"&&c.gpu.verified&&c.gpu.measuredWallSeconds>=c.gpuTargetSeconds?"PASS":c.gpu.state=="failed"?"FAIL":"INCOMPLETE");if(c.phase=="completed"&&c.cpu.verified&&c.gpu.verified&&c.gpu.latency.mean()>0)ImGui::Text("Measured CPU/GPU latency ratio: %.3fx",c.cpu.latency.mean()/c.gpu.latency.mean());}
   if(ImGui::Button("Copy compute diagnostics")){std::string text="Adapter: "+device.gpu+"\nStatus: "+device.compute.providerStatus+"\nRuntime: "+device.compute.runtimePath;const ComputeTest* items[]={&device.compute.cuda,&device.compute.cpu,&device.compute.gpu};const char* names[]={"CUDA workload","CPU inference","CUDA EP"};for(int i=0;i<3;++i)text+="\n"+std::string(names[i])+": "+items[i]->state+" ["+items[i]->reason+"] "+items[i]->message;text+="\nReport: "+device.computeReport;ImGui::SetClipboardText(text.c_str());}
   if(ImGui::CollapsingHeader("Compute results / latency comparison")){
    ImGui::TextWrapped("Loaded runtime: %s",device.compute.runtimePath.c_str());for(const auto& d:device.compute.dependencies)ImGui::TextWrapped("%s: %s / %s",d.name.c_str(),d.state.c_str(),d.message.c_str());
    const ComputeTest* results[]={&device.compute.cuda,&device.compute.cpu,&device.compute.gpu};const char* labels[]={"CUDA vector-add","ONNX CPU","ONNX CUDA EP"};
    for(int i=0;i<3;++i){const auto& r=*results[i];ImGui::Text("%s: %s",labels[i],r.state.c_str());ImGui::TextWrapped("%s",r.message.c_str());if(r.latency.count)ImGui::Text("Mean Run latency %.4f ms; serial equivalent %.1f /s (%llu runs)",r.latency.mean(),r.latency.mean()>0?1000./r.latency.mean():0.,static_cast<unsigned long long>(r.latency.count));if(r.kernel.count)ImGui::Text("CUDA event kernel mean: %.4f ms",r.kernel.mean());}
    for(const auto& item:device.compute.renderIntervals)if(item.second.count)ImGui::Text("Render FPS during %s: %.1f (%llu intervals)",item.first.c_str(),1000./item.second.mean(),static_cast<unsigned long long>(item.second.count));
    if(!device.compute.error.empty())ImGui::TextWrapped("%s",device.compute.error.c_str());if(!device.computeReport.empty())ImGui::TextWrapped("Report: %s",device.computeReport.c_str());
   }
   if(device.processUtilizationValid)ImGui::Text("Process GPU: %.2f%% (busiest engine)",device.processUtilization);else ImGui::TextDisabled("Process GPU usage: unavailable / warming up");
   ImGui::TextWrapped("%s",device.processCounterStatus.c_str());

   ImGui::Text("FPS: %.1f",ImGui::GetIO().Framerate);ImGui::Text("Frame interval: %.3f ms (includes pacing)",stats.wallFrameMs);
   if(device.utilizationValid)ImGui::Text("GPU utilization: %u%% (device-wide)",device.utilization);else ImGui::TextDisabled("GPU utilization: unavailable");
   if(device.memoryValid)ImGui::Text("Device VRAM: %.1f / %.1f MiB",device.usedBytes/1048576.,device.totalBytes/1048576.);else ImGui::TextDisabled("Device VRAM usage: unavailable");
   if(device.processMemoryValid)ImGui::Text("Process local VRAM: %.1f MiB / budget %.1f MiB",device.processVram/1048576.,device.vramBudget/1048576.);
   if(system.memoryValid)ImGui::Text("Process RAM: %.1f MiB working / %.1f MiB private",system.workingSet/1048576.,system.privateBytes/1048576.);
   if(stats.processingMs>=0)ImGui::Text("Source processing: %.3f ms / delivery: %.3f ms",stats.processingMs,stats.deliveryMs);else ImGui::TextDisabled("Source latency: awaiting sample");
   ImGui::TextWrapped("DirectX feature level: %s",device.featureLevel.c_str());ImGui::TextWrapped("%s",device.cudaStatus.c_str());ImGui::TextWrapped("%s",device.nvmlStatus.c_str());ImGui::Text("Previous frame CPU work: %.3f ms",stats.frameMs);if(system.cpuValid)ImGui::Text("Process CPU: %.2f%% (all logical cores)",system.cpu());else ImGui::TextDisabled("Process CPU: warming up / unavailable");
   ImGui::TextWrapped("GPU: %s",device.gpu.c_str());ImGui::Text("Dedicated video memory: %.0f MiB",double(device.videoMemory)/1048576.);if(device.gpuTimingValid)ImGui::Text("GPU draw time: %.3f ms (async)",device.gpuMs);else ImGui::TextDisabled("GPU timing: warming up / unavailable");ImGui::Text("Present: %s",device.flipModel?"flip-discard, low-latency":"compatibility discard");ImGui::Text("Application uptime: %.1f s",system.uptime());
   ImGui::Separator();ImGui::Text("Previous frame draw calls: %d",stats.drawCalls);ImGui::Text("Vertices: %d / indices: %d",stats.vertices,stats.indices);ImGui::Text("Entities: %d drawn / %d filtered",visuals.drawn,visuals.filtered);
  }else if(tab_==4){
   ImGui::TextWrapped("External gameplay capture / Windows Media Foundation");
   if(ImGui::CollapsingHeader("OBS setup instructions"))ImGui::TextWrapped("In OBS use Display Capture or Window Capture (Windows Graphics Capture), then Start Virtual Camera. No Game Capture hooks are used by this application.");
   if(!device.capture.running){if(ImGui::Button("Start OBS Virtual Camera"))captureRequest_=1;}
   ImGui::InputText("Local recording path",recordingPath_,sizeof(recordingPath_));
   if(!device.capture.running&&ImGui::Button("Open recording"))captureRequest_=2;
   if(device.capture.running&&ImGui::Button("Stop capture"))captureRequest_=3;
   ImGui::TextWrapped("%s: %s",device.capture.state.c_str(),device.capture.message.c_str());
   ImGui::TextWrapped("Source: %s",device.capture.source.c_str());
   if(ImGui::Button("Copy capture diagnostics")){auto text="Source: "+device.capture.source+"\nState: "+device.capture.state+"\nMessage: "+device.capture.message+"\nFrames: "+std::to_string(device.capture.frames)+"\nLog: "+Log::path().u8string();ImGui::SetClipboardText(text.c_str());}
   ImGui::Text("%ux%u | %.1f capture FPS | read wait %.2f ms",device.capture.width,device.capture.height,device.capture.fps,device.capture.readWaitMs);
   ImGui::Text("Luma %.3f | frame activity %.3f",device.capture.statistics.meanLuma,device.capture.statistics.meanAbsoluteChange);
   if(device.captureTexture&&device.capture.width){float w=std::max(1.f,std::min(ImGui::GetContentRegionAvail().x,180.f*device.capture.width/device.capture.height));ImGui::Image(static_cast<ImTextureID>(device.captureTexture),{w,w*device.capture.height/device.capture.width});}
   ImGui::TextWrapped("Preview and frame activity use real captured pixels. No semantic player detection is configured. A compatible authorized vision model and preprocessing contract are required for that next stage.");
  }else{
   ImGui::TextWrapped("Configuration: %s",Config::defaultPath().u8string().c_str());ImGui::TextWrapped("Log: %s",Log::path().u8string().c_str());
   if(ImGui::Button("Save configuration")){std::string error;notice_=Config::save(Config::defaultPath(),s,error)?"Configuration saved.":error;}
   if(ImGui::Button("Load configuration")){std::string error;notice_=Config::load(Config::defaultPath(),s,error)?"Configuration loaded.":error;}
   if(ImGui::Button("Restore defaults")){s=Settings{};notice_="Defaults restored in memory. Save to persist.";}
   ImGui::TextWrapped("%s",notice_.c_str());
  }
  ImGui::EndChild();ImGui::Separator();ImGui::TextDisabled("Local data lab | DX11 | ImGui | Insert: menu | F10: exit");
 }
 ImGui::End();ImGui::PopStyleVar();s.clamp();
}
}
