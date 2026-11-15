#include "apex/menu.hpp"
#include "apex/config.hpp"
#include "apex/log.hpp"
#include <imgui.h>
#include <algorithm>
#include <cmath>
namespace apex { namespace {
const ImVec4 muted{.56f,.60f,.70f,1},good{.38f,.82f,.67f,1},warning{.96f,.73f,.37f,1};
void hint(const char* text){ImGui::PushStyleColor(ImGuiCol_Text,muted);ImGui::TextWrapped("%s",text);ImGui::PopStyleColor();}
void section(const char* title,const char* description=nullptr){ImGui::Spacing();ImGui::TextUnformatted(title);if(description&&*description)hint(description);ImGui::Spacing();}
void statCard(const char* id,const char* heading,const char* value,const char* detail,ImVec4 color){ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{13,12});ImGui::BeginChild(id,{0,104*ImGui::GetIO().FontGlobalScale},true);ImGui::TextDisabled("%s",heading);ImGui::PushStyleColor(ImGuiCol_Text,color);ImGui::TextWrapped("%s",value);ImGui::PopStyleColor();if(ImGui::GetContentRegionAvail().x>150)hint(detail);else if(ImGui::IsWindowHovered())ImGui::SetTooltip("%s",detail);ImGui::EndChild();ImGui::PopStyleVar();}
bool stepCard(const char* id,const char* number,const char* heading,const char* description,const char* button,bool complete){ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{14,12});ImGui::BeginChild(id,{0,(ImGui::GetContentRegionAvail().x<550?112.f:92.f)*ImGui::GetIO().FontGlobalScale},true);ImGui::TextColored(complete?good:muted,"%s",number);ImGui::SameLine();ImGui::TextUnformatted(heading);float right=ImGui::GetWindowContentRegionMax().x-112;bool clicked=false;if(right>ImGui::GetCursorPosX()+80)ImGui::SameLine(right);if(ImGui::Button(button,{112,0}))clicked=true;hint(description);ImGui::EndChild();ImGui::PopStyleVar();return clicked;}
bool nav(const char* label,int page,int& current,ImVec4 accent){bool selected=current==page;auto origin=ImGui::GetCursorScreenPos();if(selected)ImGui::PushStyleColor(ImGuiCol_Header,{accent.x*.25f,accent.y*.25f,accent.z*.25f,1});bool clicked=ImGui::Selectable(label,selected,0,{0,32*ImGui::GetIO().FontGlobalScale});if(selected){ImGui::GetWindowDrawList()->AddRectFilled({origin.x-5,origin.y+5},{origin.x-2,origin.y+27},ImGui::ColorConvertFloat4ToU32(accent),2);ImGui::PopStyleColor();}if(clicked)current=page;return clicked;}
}
void Menu::draw(Settings& s,const DeviceInfo& device,const SystemMetrics& system,const RenderStats& stats,const VisualStats& visuals,bool shown,float dt,const std::string& sourceStatus){
 fade_+=(float(shown)-fade_)*(1-std::exp(-dt*15));if(!shown&&fade_<.01f)return;
 ImGuiStyle& style=ImGui::GetStyle();style.WindowRounding=13;style.ChildRounding=9;style.FrameRounding=6;style.WindowBorderSize=1;style.FrameBorderSize=0;style.FramePadding={10,7};style.ItemSpacing={10,8};style.ScrollbarSize=10;style.ScrollbarRounding=6;
 const ImVec4 accent{s.accent[0],s.accent[1],s.accent[2],1};
 style.Colors[ImGuiCol_Text]={.91f,.93f,.97f,1};style.Colors[ImGuiCol_TextDisabled]=muted;style.Colors[ImGuiCol_WindowBg]={.028f,.034f,.049f,1};style.Colors[ImGuiCol_ChildBg]={.043f,.052f,.074f,1};style.Colors[ImGuiCol_Border]={.13f,.15f,.21f,1};style.Colors[ImGuiCol_Separator]=style.Colors[ImGuiCol_Border];
 style.Colors[ImGuiCol_FrameBg]={.075f,.088f,.122f,1};style.Colors[ImGuiCol_FrameBgHovered]={.11f,.12f,.17f,1};style.Colors[ImGuiCol_FrameBgActive]={.14f,.14f,.22f,1};style.Colors[ImGuiCol_Button]={.115f,.105f,.19f,1};style.Colors[ImGuiCol_ButtonHovered]={accent.x*.55f,accent.y*.55f,accent.z*.55f,1};style.Colors[ImGuiCol_ButtonActive]=accent;style.Colors[ImGuiCol_CheckMark]=accent;style.Colors[ImGuiCol_SliderGrab]=accent;style.Colors[ImGuiCol_SliderGrabActive]={.8f,.65f,1,1};style.Colors[ImGuiCol_Header]={.09f,.10f,.14f,1};style.Colors[ImGuiCol_HeaderHovered]={.15f,.13f,.22f,1};style.Colors[ImGuiCol_HeaderActive]={.2f,.16f,.3f,1};style.Colors[ImGuiCol_Tab]={.065f,.075f,.1f,1};style.Colors[ImGuiCol_TabSelected]={.16f,.12f,.25f,1};style.Colors[ImGuiCol_TabHovered]=style.Colors[ImGuiCol_ButtonHovered];
 auto screen=ImGui::GetIO().DisplaySize;float scale=ImGui::GetIO().FontGlobalScale;
 ImGui::SetNextWindowSize({std::min(s.menuWidth*scale,screen.x-24),std::min(s.menuHeight*scale,screen.y-24)},ImGuiCond_Always);ImGui::SetNextWindowPos({screen.x*.5f,screen.y*.5f},ImGuiCond_Always,{.5f,.5f});
 ImGui::PushStyleVar(ImGuiStyleVar_Alpha,fade_);ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{18,16});
 ImGuiWindowFlags flags=ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoTitleBar;if(!shown)flags|=ImGuiWindowFlags_NoInputs;
 if(ImGui::Begin("Apex Visualization",nullptr,flags)){
  ImGui::TextColored(accent,"AV");ImGui::SameLine();ImGui::TextUnformatted("APEX VISION");ImGui::SameLine();ImGui::TextDisabled("/ WORKSPACE");
  if(ImGui::GetWindowWidth()>650*scale){ImGui::SameLine(ImGui::GetWindowContentRegionMax().x-135*scale);if(ImGui::SmallButton("Expand layout")){s.menuWidth=1000;s.menuHeight=740;}}
  ImGui::Spacing();ImGui::Separator();ImGui::Spacing();
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{12,12});ImGui::BeginChild("Sidebar",{155*scale,-32},true);
  ImGui::TextDisabled("WORKSPACE");ImGui::Spacing();nav("Home",7,tab_,accent);nav("Capture",4,tab_,accent);nav("Detection",5,tab_,accent);nav("Tracking",6,tab_,accent);nav("Screen overlay",8,tab_,accent);
  ImGui::Spacing();ImGui::Separator();ImGui::Spacing();ImGui::TextDisabled("MANAGE");ImGui::Spacing();nav("Appearance",1,tab_,accent);nav("System",2,tab_,accent);nav("Settings",3,tab_,accent);
  ImGui::Spacing();ImGui::Separator();ImGui::Spacing();ImGui::TextDisabled("ADVANCED");ImGui::Spacing();nav("Data lab",0,tab_,accent);ImGui::EndChild();ImGui::PopStyleVar();ImGui::SameLine();
  ImGui::BeginChild("Page",{0,-32},false);ImGui::PushItemWidth(std::max(110.f,ImGui::GetContentRegionAvail().x*.58f));
  const char* titles[]={"Data lab","Appearance","System","Settings","Capture","Detection","Tracking","Home","Screen overlay"};tab_=std::clamp(tab_,0,8);
  const char* subtitles[]={"Local JSON and simulated visualization tools.","Theme, window layout and rendering preferences.","Live performance and optional compute checks.","Saved preferences and application files.","Connect a video source before starting detection.","Choose a model and inspect its captured results.","Configure the window, axes and tracking circle.","A clear path from capture to your workspace.","Draw on your actual monitor, independently of the preview."};
  ImGui::TextUnformatted(titles[tab_]);hint(subtitles[tab_]);ImGui::Spacing();ImGui::Separator();ImGui::Spacing();
  if(tab_==7){
   bool captured=device.capture.running&&device.capture.frame;bool configured=!enemyOptions.modelPath.empty();bool modelReady=device.enemy.completed>0&&device.enemy.status!="unavailable";bool cuda=device.enemy.latest.cudaVerified;
   int columns=ImGui::GetContentRegionAvail().x<420?1:3;
   if(ImGui::BeginTable("HomeStatus",columns,ImGuiTableFlags_SizingStretchSame)){ImGui::TableNextColumn();statCard("CaptureCard","CAPTURE",captured?(device.capture.isRecording?"Recording":"Live"):"Not connected",captured?"Frames received":"Connect OBS or a recording",captured?good:muted);ImGui::TableNextColumn();statCard("ModelCard","DETECTION",device.enemy.status=="unavailable"?"Needs attention":modelReady?"Results received":configured?"Configured":"No model",enemyOptions.profile==EnemyProfile::Apex416?"Apex profile":"Roblox profile",device.enemy.status=="unavailable"?warning:modelReady?good:muted);ImGui::TableNextColumn();statCard("EngineCard","LAST MODEL RUN",cuda?"CUDA profiled":device.enemy.latest.backend=="ONNX CPU"?"CPU":"Not verified",cuda?"CPU operators may also run":"Last detector result",cuda?good:muted);ImGui::EndTable();}
   if(ImGui::Button("Show overlay on my screen")){overlayOptions.enabled=true;overlayRequest_=1;tab_=8;}ImGui::SameLine();if(ImGui::Button("Overlay settings"))tab_=8;
   section("Get started", "Complete these steps in order. Existing model and tracking preferences stay separate.");
   if(stepCard("StepCapture","01","Connect capture",captured?"Your video source has delivered frames. Open Capture to inspect it.":"Start OBS Virtual Camera, then connect it here.","Capture",captured))tab_=4;
   if(stepCard("StepModel","02","Choose a detector",configured?"Your model path is configured. Enable analysis and check the preview.":"Select the Apex or Roblox profile and choose your local ONNX file.","Detection",modelReady))tab_=5;
   if(stepCard("StepTracking","03","Configure tracking",device.trackingWindowState.valid?"A window is selected. Review axes and FOV before explicitly arming.":"Select the Apex window, adjust X / Y and enable the FOV circle.","Tracking",device.trackingWindowState.valid))tab_=6;
   ImGui::Spacing();hint("F8  overlay visibility     F9  disarm tracking     Insert  menu     F10  exit");
   if(ImGui::CollapsingHeader("Need help?")){hint("Start with a moving Capture preview. Detection needs a compatible local model and incoming-frame analysis enabled. Tracking additionally needs the Apex profile, a selected game window, F9 available and a fresh live result. Right mouse is a hold control; arming is never restored on restart.");if(ImGui::Button("Open system checks")){tab_=2;systemTab_=1;}}
  }else if(tab_==0){

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
   section("Display", "Use the frame cap to control UI rendering load.");ImGui::Checkbox("VSync",&s.vsync);ImGui::SliderInt("Frame cap",&s.frameLimit,30,360);
   section("Window and theme");if(ImGui::Button("Compact layout")){s.menuWidth=760;s.menuHeight=600;}ImGui::SameLine();if(ImGui::Button("Comfortable layout##Appearance")){s.menuWidth=1000;s.menuHeight=740;}
   ImGui::SliderFloat("Menu width",&s.menuWidth,560,1200,"%.0f px");ImGui::SliderFloat("Menu height",&s.menuHeight,440,900,"%.0f px");ImGui::ColorEdit3("Accent color",s.accent);hint("Layout and theme are saved with Settings > Save configuration.");
   if(ImGui::CollapsingHeader("Data lab animation")){ImGui::Checkbox("Animate simulated scene",&s.animate);ImGui::SliderFloat("Playback speed",&s.playbackSpeed,.1f,3,"%.1fx");const char* scenes[]={"Arena patrol","Tracking lanes","Static inspection"};ImGui::Combo("Training scene",&s.scene,scenes,3);if(ImGui::Button("Reset simulation (F6)"))reset_=true;hint("F5 pauses the simulated scene. F6 resets it.");}
  }else if(tab_==2){
   if(ImGui::Button(systemTab_==0?"[ Performance ]":"Performance"))systemTab_=0;ImGui::SameLine();if(ImGui::Button(systemTab_==1?"[ Compute checks ]":"Compute checks"))systemTab_=1;ImGui::Spacing();
   if(systemTab_==1){section("Compute validation", "These checks are separate from your live detection model.");

   ImGui::TextWrapped("Compute status: %s",device.compute.providerStatus.c_str());
   if(!device.compute.error.empty())ImGui::TextWrapped("ERROR: %s",device.compute.error.c_str());
   if(device.compute.running){ImGui::TextWrapped("Running: %s",device.compute.phase.c_str());if(ImGui::Button("Cancel compute test"))computeRequest_=3;}
   else {if(ImGui::Button("Run CUDA + ONNX validation"))computeRequest_=1;if(ImGui::Button("Sustained test (5 min per available phase)"))computeRequest_=2;}
   if(ImGui::Button("Use bundled test model")){modelPath_[0]=0;inputProfile_[0]=0;}
   ImGui::InputText("Model path (blank = bundled)",modelPath_,sizeof(modelPath_));
   ImGui::InputText("Input shape JSON (optional)",inputProfile_,sizeof(inputProfile_));
   ImGui::TextWrapped("FP32 tensors; dynamic dimensions require a shape profile. Ctrl+V supported. Paths apply to model benchmarks only.");
   if(!device.compute.running){if(ImGui::Button("Benchmark loaded model CPU / GPU"))computeRequest_=4;if(ImGui::Button("5-minute GPU model test"))computeRequest_=5;}
   if(!device.compute.modelPath.empty())ImGui::TextWrapped("Model: %s\nSHA256: %s",device.compute.modelPath.c_str(),device.compute.modelSha256.c_str());
   for(const auto* r:{&device.compute.cpu,&device.compute.gpu})if(r->latency.count){ImGui::Text("%s: %.4f ms Run; %.1f inference FPS (%llu runs)",r==&device.compute.cpu?"CPU":"GPU",r->latency.mean(),r->measuredWallSeconds>0?r->latency.count/r->measuredWallSeconds:0.,static_cast<unsigned long long>(r->latency.count));}
   if(device.compute.modelBenchmark){const auto& r=device.compute.gpu;ImGui::Text("GPU measured %.1f / %.1f seconds; %s",std::max(0.,r.measuredWallSeconds),device.compute.gpuTargetSeconds,r.state.c_str());for(const auto* trend:{&device.compute.vramTrend,&device.compute.privateTrend}){ImGui::Text("%s: %s, growth %.1f MiB, %.2f MiB/min",trend==&device.compute.vramTrend?"VRAM trend":"Private RAM trend",trend->state.c_str(),trend->growthMiB,trend->slopeMiBPerMinute);ImGui::TextWrapped("%s",trend->message.c_str());}}
   if(device.compute.finished){const auto& c=device.compute;ImGui::Text("GPU execution: %s",c.phase=="completed"&&c.gpu.verified?"PASS":c.gpu.state=="failed"?"FAIL":"INCOMPLETE");if(c.modelBenchmark&&c.gpuTargetSeconds>=300)ImGui::Text("Five-minute GPU execution: %s",c.phase=="completed"&&c.gpu.verified&&c.gpu.measuredWallSeconds>=c.gpuTargetSeconds?"PASS":c.gpu.state=="failed"?"FAIL":"INCOMPLETE");if(c.phase=="completed"&&c.cpu.verified&&c.gpu.verified&&c.gpu.latency.mean()>0)ImGui::Text("Measured CPU/GPU latency ratio: %.3fx",c.cpu.latency.mean()/c.gpu.latency.mean());}
   if(ImGui::Button("Copy compute diagnostics")){std::string text="Adapter: "+device.gpu+"\nStatus: "+device.compute.providerStatus+"\nRuntime: "+device.compute.runtimePath+"\nPhase: "+device.compute.phase+"\nError: "+device.compute.error+"\nModel: "+device.compute.modelPath+"\nInput profile: "+device.compute.inputProfilePath;const ComputeTest* items[]={&device.compute.cuda,&device.compute.cpu,&device.compute.gpu};const char* names[]={"CUDA workload","CPU inference","CUDA EP"};for(int i=0;i<3;++i)text+="\n"+std::string(names[i])+": "+items[i]->state+" ["+items[i]->reason+"] "+items[i]->message;text+="\nReport: "+device.computeReport;ImGui::SetClipboardText(text.c_str());}
   if(ImGui::CollapsingHeader("Compute results / latency comparison")){
    ImGui::TextWrapped("Loaded runtime: %s",device.compute.runtimePath.c_str());for(const auto& d:device.compute.dependencies)ImGui::TextWrapped("%s: %s / %s",d.name.c_str(),d.state.c_str(),d.message.c_str());
    const ComputeTest* results[]={&device.compute.cuda,&device.compute.cpu,&device.compute.gpu};const char* labels[]={"CUDA vector-add","ONNX CPU","ONNX CUDA EP"};
    for(int i=0;i<3;++i){const auto& r=*results[i];ImGui::Text("%s: %s",labels[i],r.state.c_str());ImGui::TextWrapped("%s",r.message.c_str());if(r.latency.count)ImGui::Text("Mean Run latency %.4f ms; serial equivalent %.1f /s (%llu runs)",r.latency.mean(),r.latency.mean()>0?1000./r.latency.mean():0.,static_cast<unsigned long long>(r.latency.count));if(r.kernel.count)ImGui::Text("CUDA event kernel mean: %.4f ms",r.kernel.mean());}
    for(const auto& item:device.compute.renderIntervals)if(item.second.count)ImGui::Text("Render FPS during %s: %.1f (%llu intervals)",item.first.c_str(),1000./item.second.mean(),static_cast<unsigned long long>(item.second.count));
    if(!device.compute.error.empty())ImGui::TextWrapped("%s",device.compute.error.c_str());if(!device.computeReport.empty())ImGui::TextWrapped("Report: %s",device.computeReport.c_str());
   }
   }else{
    std::string fps=stats.wallFrameMs>0?std::to_string(int(std::round(1000./stats.wallFrameMs)))+" FPS":"Warming up";std::string cpu=system.cpuValid?std::to_string(int(std::round(system.cpu())))+"%":"Warming up";std::string gpu=device.processUtilizationValid?std::to_string(int(std::round(device.processUtilization)))+"%":"Unavailable";
    if(ImGui::BeginTable("PerformanceCards",ImGui::GetContentRegionAvail().x<420?1:3,ImGuiTableFlags_SizingStretchSame)){ImGui::TableNextColumn();statCard("Fps","UI RENDER",fps.c_str(),"Includes frame pacing",muted);ImGui::TableNextColumn();statCard("Cpu","PROCESS CPU",cpu.c_str(),"All logical cores",muted);ImGui::TableNextColumn();statCard("Gpu","PROCESS GPU",gpu.c_str(),"Busiest engine",muted);ImGui::EndTable();}
    section("Hardware");ImGui::TextWrapped("%s",device.gpu.c_str());if(device.processMemoryValid)ImGui::Text("Local VRAM %.1f MiB / budget %.1f MiB",device.processVram/1048576.,device.vramBudget/1048576.);if(system.memoryValid)ImGui::Text("RAM %.1f MiB working / %.1f MiB private",system.workingSet/1048576.,system.privateBytes/1048576.);
    if(ImGui::CollapsingHeader("Detailed performance counters")){
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

    }
   }
  }else if(tab_==4){
   if(!device.capture.running){if(ImGui::Button("Connect OBS camera",{180,0}))captureRequest_=1;if(ImGui::GetContentRegionAvail().x>190)ImGui::SameLine();if(ImGui::Button("Choose recording",{180,0}))captureRequest_=4;}
   else {if(ImGui::Button("Disconnect",{140,0}))captureRequest_=3;if(device.capture.isRecording){ImGui::SameLine();if(ImGui::Button(device.capture.paused?"Resume":"Pause",{110,0}))captureRequest_=5;}}
   ImGui::TextColored(device.capture.running?good:muted,"%s",device.capture.running?(device.capture.isRecording?"RECORDING SOURCE":"LIVE CAMERA"):"NOT CONNECTED");
   if(device.captureTexture&&device.capture.width&&device.capture.height){float width=std::max(1.f,std::min(ImGui::GetContentRegionAvail().x,220.f*device.capture.width/device.capture.height));ImGui::Image(static_cast<ImTextureID>(device.captureTexture),{width,width*device.capture.height/device.capture.width});}
   ImGui::TextWrapped("%s",device.capture.message.c_str());ImGui::Text("%ux%u  /  %.1f capture FPS  /  %llu frames",device.capture.width,device.capture.height,device.capture.fps,static_cast<unsigned long long>(device.capture.frames));
   if(ImGui::Button("Continue to Detection"))tab_=5;
   if(ImGui::CollapsingHeader("OBS setup"))hint("In OBS choose Window Capture or Display Capture of the full game view, then Start Virtual Camera. Return here and Connect OBS camera. This app does not add Game Capture hooks.");
   if(ImGui::CollapsingHeader("Recording path")){ImGui::InputText("Local file",recordingPath_,sizeof(recordingPath_));ImGui::BeginDisabled(device.capture.running);if(ImGui::Button("Open recording path"))captureRequest_=2;ImGui::EndDisabled();}
   if(ImGui::CollapsingHeader("Capture diagnostics")){ImGui::TextWrapped("Source: %s",device.capture.source.c_str());ImGui::Text("Read wait %.2f ms | luma %.3f | activity %.3f",device.capture.readWaitMs,device.capture.statistics.meanLuma,device.capture.statistics.meanAbsoluteChange);if(ImGui::Button("Copy capture diagnostics")){auto text="Source: "+device.capture.source+"\nState: "+device.capture.state+"\nMessage: "+device.capture.message+"\nFrames: "+std::to_string(device.capture.frames)+"\nLog: "+Log::path().u8string();ImGui::SetClipboardText(text.c_str());}}
  }else if(tab_==5){

   ImGui::SetNextItemOpen(!showTextTools,ImGuiCond_Once);
   if(ImGui::CollapsingHeader("Model and live preview")){

   if(ImGui::Checkbox("Show overlay on my screen",&overlayOptions.enabled))overlayRequest_=1;ImGui::SameLine();if(ImGui::Button("Screen settings"))tab_=8;
   int selectedProfile=static_cast<int>(enemyOptions.profile);const char* profileNames[]={"Apex (primary, 416px)","Roblox (test, YOLO 640px)"};
   if(ImGui::Combo("Model profile",&selectedProfile,profileNames,2)){std::string error;auto root=Config::defaultPath().parent_path();if(!saveEnemyOptions(enemySettingsFile(root,enemyOptions.profile),enemyOptions,error))notice_=error;else{EnemyOptions next=enemyOptions;next.profile=static_cast<EnemyProfile>(selectedProfile);next.modelPath.clear();next.confidence=.5f;next.iou=.45f;next.intervalMs=150;auto path=enemySettingsFile(root,next.profile);if(std::filesystem::exists(path)&&!loadEnemyOptions(path,next,error))notice_=error;else{enemyOptions=next;enemyEnabled=false;enemyRequest_=4;notice_="Profile changed; previous model path/settings saved separately. Start capture and enable analysis.";}}}

   if(device.capture.running&&device.capture.isRecording&&ImGui::Button(device.capture.paused?"Resume analysis clip":"Pause analysis clip"))captureRequest_=5;
   if(ImGui::Button("Choose ONNX model",{180,0}))enemyRequest_=3;
   ImGui::SameLine();ImGui::TextDisabled("%s",enemyOptions.modelPath.empty()?"No model selected":enemyOptions.modelPath.filename().u8string().c_str());if(ImGui::IsItemHovered()&&!enemyOptions.modelPath.empty())ImGui::SetTooltip("%s",enemyOptions.modelPath.u8string().c_str());
   ImGui::BeginDisabled(enemyOptions.modelPath.empty()||!device.capture.frame);ImGui::Checkbox("Enable detection",&enemyEnabled);if(ImGui::Button("Analyze current frame"))enemyRequest_=1;ImGui::EndDisabled();
   if(enemyOptions.modelPath.empty()||!device.capture.frame)hint("Choose a model and connect Capture to enable analysis.");
   if(ImGui::Button("Low-latency live preset")){enemyOptions.intervalMs=16;enemyOptions.maxResultAgeMs=200;enemyOptions.preferCuda=true;enemyPreviewAnalyzed=false;notice_="Requested CUDA and 16 ms scheduling; live boxes expire at 200 ms frame age. Actual rate depends on capture/model; use Save enemy setup to persist.";}
   ImGui::Checkbox("Exact analyzed frame (slower inspection view)",&enemyPreviewAnalyzed);
   const bool exact=enemyPreviewAnalyzed||!device.capture.frame;
   auto previewFrame=exact?device.enemy.analyzedFrame:device.capture.frame;auto previewTexture=exact?device.enemyTexture:device.captureTexture;
   const double resultAge=enemyFrameAgeMs(device.enemy)/1000.;const bool freshResult=enemyResultFresh(device.enemy,enemyOptions.maxResultAgeMs);
   if(previewFrame&&previewTexture){auto frame=previewFrame;float w=std::min(ImGui::GetContentRegionAvail().x,190.f*frame->width/frame->height);ImVec2 size{w,w*frame->height/frame->width};auto origin=ImGui::GetCursorScreenPos();ImGui::Image(static_cast<ImTextureID>(previewTexture),size);if(exact||freshResult)for(const auto& box:device.enemy.latest.boxes){auto* draw=ImGui::GetWindowDrawList();ImVec2 a{origin.x+box.x1*size.x,origin.y+box.y1*size.y},b{origin.x+box.x2*size.x,origin.y+box.y2*size.y};draw->AddRect(a,b,IM_COL32(255,90,110,255),0,0,2);char text[64];snprintf(text,sizeof(text),"Enemy? %.2f",box.score);draw->AddText(a,IM_COL32(255,220,220,255),text);}if(exact)ImGui::Text("Exact analyzed frame %llu | %zu candidates | %.1f ms",static_cast<unsigned long long>(device.enemy.latest.sequence),device.enemy.latest.boxes.size(),device.enemy.latest.latencyMs);else{ImGui::Text("Live capture %.1f FPS | frame %llu",device.capture.fps,static_cast<unsigned long long>(frame->sequence));if(device.enemy.completed)ImGui::Text("Detection frame %llu | age %.0f ms | processing %.1f ms",static_cast<unsigned long long>(device.enemy.latest.sequence),resultAge*1000,device.enemy.latest.latencyMs);else ImGui::TextDisabled("Live preview continues while the model initializes / analyzes");}}
   if(ImGui::CollapsingHeader("Desktop overlay and monitor")){
   ImGui::Checkbox("Draw detected boxes on screen",&overlayOptions.showBoxes);
   if(ImGui::BeginCombo("Overlay monitor",device.overlayMonitors.empty()?"No monitor":device.overlayMonitors[std::clamp(overlayOptions.monitor,0,int(device.overlayMonitors.size())-1)].c_str())){for(int n=0;n<int(device.overlayMonitors.size());++n)if(ImGui::Selectable(device.overlayMonitors[n].c_str(),overlayOptions.monitor==n))overlayOptions.monitor=n;ImGui::EndCombo();}
   ImGui::Checkbox("Fit capture aspect ratio to monitor",&overlayOptions.fitAspect);
   ImGui::TextWrapped("%s",device.overlayStatus.c_str());ImGui::TextWrapped("%s",device.overlayHotkey?"F8: show/hide overlay, even while the game has focus":"F8 unavailable (already reserved); use the overlay checkbox");
   ImGui::TextWrapped("Use borderless/windowed gameplay on the selected monitor. OBS must show that same full game view; scene cropping or borders can misalign boxes.");
   }
   if(ImGui::CollapsingHeader("Detection tuning and saving")){hint("The low-latency preset requests faster scheduling. Actual capture/inference speed must be measured; it does not make a CPU-only runtime use CUDA.");
   ImGui::Checkbox("Prefer CUDA for enemy model",&enemyOptions.preferCuda);ImGui::SliderFloat("Detection score",&enemyOptions.confidence,.05f,.95f,"%.2f");ImGui::SliderFloat("Overlap suppression",&enemyOptions.iou,.1f,.9f,"%.2f");ImGui::SliderInt("Detection interval ms",&enemyOptions.intervalMs,16,2000);ImGui::SliderInt("Maximum live box age ms",&enemyOptions.maxResultAgeMs,50,1000);hint("Age starts when a frame reaches this receiver, including queue and inference time. OBS sender delay is unknown. Old boxes clear rather than trailing targets; exact-frame inspection keeps its matching boxes.");

   if(ImGui::Button("Save enemy setup")){std::string error;notice_=saveEnemyOptions(enemySettingsFile(Config::defaultPath().parent_path(),enemyOptions.profile),enemyOptions,error)?"Enemy setup saved.":error;}
   }
   ImGui::TextWrapped("%s",notice_.c_str());
   ImGui::TextWrapped("%s: %s",device.enemy.status.c_str(),device.enemy.message.c_str());ImGui::TextWrapped("Backend: %s",device.enemy.latest.backend.c_str());if(device.enemy.completed){if(!enemyPreviewAnalyzed&&!freshResult)ImGui::TextColored(warning,"Live boxes expired: source frame is too old (limit %d ms)",enemyOptions.maxResultAgeMs);if(enemyOptions.preferCuda&&!device.enemy.latest.cudaVerified)hint("CUDA execution is not verified for this model. Check diagnostics; CPU fallback can cause lag.");}

   if(ImGui::CollapsingHeader("Model details and diagnostics")){
   ImGui::TextWrapped("Original model class labels: %s",device.enemy.latest.modelLabels.c_str());ImGui::TextWrapped("SHA256: %s",device.enemy.latest.modelSha.c_str());
   if(ImGui::Button("Copy detection diagnostics")){auto text="Profile: "+std::string(enemyOptions.profile==EnemyProfile::Apex416?"Apex416":"Roblox640")+"\nStatus: "+device.enemy.status+"\nMessage: "+device.enemy.message+"\nBackend: "+device.enemy.latest.backend+"\nModel: "+enemyOptions.modelPath.u8string()+"\nModel SHA256: "+device.enemy.latest.modelSha+"\nOriginal labels: "+device.enemy.latest.modelLabels+"\nCandidates: "+std::to_string(device.enemy.latest.boxes.size())+"\nCapture FPS: "+std::to_string(device.capture.fps)+"\nModel processing ms: "+std::to_string(device.enemy.latest.latencyMs)+"\nResult age ms: "+std::to_string(resultAge*1000)+"\nDetection interval ms: "+std::to_string(enemyOptions.intervalMs)+"\nMaximum live box age ms: "+std::to_string(enemyOptions.maxResultAgeMs)+"\nUI frame ms: "+std::to_string(stats.wallFrameMs);ImGui::SetClipboardText(text.c_str());}
   }
   if(ImGui::CollapsingHeader("About detection results")){
   ImGui::TextWrapped("Live preview uses the latest captured image; boxes are delayed model results. Exact analyzed-frame mode aligns boxes to their source pixels. Class 0 is an enemy candidate by your model description; friend/foe accuracy is not independently validated. No weapon detection is added.");   }

   }
   ImGui::SetNextItemOpen(showTextTools,ImGuiCond_Once);
   if(ImGui::CollapsingHeader("Recorded text reader")){
   ImGui::TextWrapped("Read visible HUD text from local recordings");
   if(ImGui::Button("Open recording..."))captureRequest_=4;ImGui::SameLine();
   if(device.capture.running&&device.capture.isRecording){if(ImGui::Button(device.capture.paused?"Resume clip":"Pause clip"))captureRequest_=5;}
   ImGui::TextWrapped("%s",device.capture.source.c_str());
   if(device.captureTexture&&device.capture.width){
    float w=std::max(1.f,std::min(ImGui::GetContentRegionAvail().x,250.f*device.capture.width/device.capture.height));ImVec2 size{w,w*device.capture.height/device.capture.width};auto origin=ImGui::GetCursorScreenPos();ImGui::Image(static_cast<ImTextureID>(device.captureTexture),size);auto& io=ImGui::GetIO();
    auto local=[&](ImVec2 mouse){return ImVec2{std::clamp((mouse.x-origin.x)/size.x,0.f,1.f),std::clamp((mouse.y-origin.y)/size.y,0.f,1.f)};};
    if(ImGui::IsItemHovered()&&ImGui::IsMouseClicked(0)){auto a=local(io.MousePos);hudAnchorX_=a.x;hudAnchorY_=a.y;hudDragging_=true;}
    if(hudDragging_){auto b=local(io.MousePos);float x=std::min(hudAnchorX_,b.x),y=std::min(hudAnchorY_,b.y),right=std::max(hudAnchorX_,b.x),bottom=std::max(hudAnchorY_,b.y);if(right-x>.002f&&bottom-y>.002f)hudOptions.region={x,y,right-x,bottom-y};if(!ImGui::IsMouseDown(0))hudDragging_=false;}
    auto r=hudOptions.region;ImGui::GetWindowDrawList()->AddRect({origin.x+r.x*size.x,origin.y+r.y*size.y},{origin.x+(r.x+r.width)*size.x,origin.y+(r.y+r.height)*size.y},IM_COL32(130,255,180,255),0,0,2);ImGui::TextWrapped("Pause the clip, then drag a box around ONLY the text to read.");
   }else ImGui::TextWrapped("Open an MP4 recording first. Existing OBS capture stays on the Capture page.");
   const char* modes[]={"Ammo counter","Match-result banner","Raw text"};int mode=static_cast<int>(hudOptions.mode);if(ImGui::Combo("Read",&mode,modes,3))hudOptions.mode=static_cast<HudMode>(mode);
   auto& r=hudOptions.region;ImGui::SliderFloat("Region X",&r.x,0,.999f,"%.3f");ImGui::SliderFloat("Region Y",&r.y,0,.999f,"%.3f");r.width=std::clamp(r.width,.001f,1-r.x);r.height=std::clamp(r.height,.001f,1-r.y);ImGui::SliderFloat("Region width",&r.width,.001f,1-r.x,"%.3f");ImGui::SliderFloat("Region height",&r.height,.001f,1-r.y,"%.3f");
   ImGui::SliderInt("Text upscale",&hudOptions.scale,1,4);ImGui::SliderInt("OCR interval (ms)",&hudOptions.intervalMs,250,5000);
   ImGui::BeginDisabled(!device.capture.isRecording||!device.capture.frame);ImGui::Checkbox("Read HUD while clip plays",&hudEnabled);if(ImGui::Button("Read current frame"))hudRequest_=1;ImGui::EndDisabled();
   if(ImGui::Button("Save HUD setup")){std::string error;notice_=saveHudOptions(Config::defaultPath().parent_path()/"hud-settings.json",hudOptions,error)?"HUD region and mode saved.":error;}ImGui::SameLine();if(ImGui::Button("Load HUD setup")){std::string error;notice_=loadHudOptions(Config::defaultPath().parent_path()/"hud-settings.json",hudOptions,error)?"HUD setup loaded.":error;}
   ImGui::TextWrapped("%s",notice_.c_str());ImGui::TextWrapped("Backend: %s",device.hud.backend.c_str());ImGui::TextWrapped("%s: %s",device.hud.status.c_str(),device.hud.message.c_str());const auto& result=device.hud.latest;
   ImGui::TextWrapped("Raw OCR: %s",result.raw.empty()?"(no text read)":result.raw.c_str());
   if(result.magazine)ImGui::Text("Magazine candidate: %u",*result.magazine);if(result.reserve)ImGui::Text("Reserve candidate: %u",*result.reserve);if(!result.banner.empty())ImGui::Text("Banner candidate: %s",result.banner.c_str());
   if(device.hud.completed)ImGui::Text("OCR %.2f ms | source frame %llu | %llu observations",result.latencyMs,static_cast<unsigned long long>(result.sequence),static_cast<unsigned long long>(device.hud.completed));
   if(ImGui::Button("Copy OCR diagnostics")){auto text="Backend: "+device.hud.backend+"\nStatus: "+device.hud.status+"\nMessage: "+device.hud.message+"\nRaw OCR: "+result.raw+"\nSource: "+device.capture.source+"\nFrame: "+std::to_string(result.sequence);ImGui::SetClipboardText(text.c_str());}
   ImGui::TextDisabled("CSV export keeps the latest 600 observations for this source.");
   ImGui::BeginDisabled(!device.hud.completed);if(ImGui::Button("Export OCR CSV..."))hudRequest_=2;ImGui::SameLine();if(ImGui::Button("Copy OCR result")){auto text="Backend: "+device.hud.backend+"\nStatus: "+device.hud.status+"\nRaw OCR: "+result.raw+"\nSource: "+result.source+"\nFrame: "+std::to_string(result.sequence)+"\nOCR ms: "+std::to_string(result.latencyMs);ImGui::SetClipboardText(text.c_str());}ImGui::EndDisabled();
   ImGui::TextWrapped("OCR reads pixels; parsed values are candidates, not ground truth. No confidence score is exposed by this backend. Windows OCR uses CPU and is separate from CUDA validation.");

   }
  }else if(tab_==6){

   section("1. Select the game window");
   ImGui::PushItemWidth(-1);if(ImGui::BeginCombo("##ApexWindow",trackingWindow.handle?trackingWindow.title.c_str():"Select the game window")){if(device.trackingWindows.empty())ImGui::TextDisabled("No Apex window found. Open the game, then Refresh.");for(size_t n=0;n<device.trackingWindows.size();++n){const auto& w=device.trackingWindows[n];ImGui::PushID(int(n));if(ImGui::Selectable(w.title.c_str(),trackingWindow.handle==w.handle)){trackingWindow=w;trackingOptions.armed=false;trackingRequest_=2;}ImGui::PopID();}ImGui::EndCombo();}ImGui::PopItemWidth();if(ImGui::Button("Refresh windows",{150,0}))trackingRequest_=1;ImGui::SameLine();ImGui::TextDisabled("%zu found | auto-refresh 1s",device.trackingWindows.size());if(device.trackingWindows.empty())hint("Open Apex first. Minimized games remain selectable; restore the selected game before tracking. Use borderless/windowed mode.");if(trackingWindow.handle)ImGui::TextWrapped("%s",device.trackingWindowState.status.c_str());
   section("2. Activation");
   bool ready=device.trackingEmergency&&device.trackingWindowState.valid&&device.capture.running&&!device.capture.isRecording&&enemyEnabled&&enemyOptions.profile==EnemyProfile::Apex416&&!enemyOptions.modelPath.empty();
   if(!device.capture.running||device.capture.isRecording)ImGui::TextWrapped("To arm: start live OBS capture (recordings cannot drive mouse input).");
   else if(enemyOptions.profile!=EnemyProfile::Apex416||enemyOptions.modelPath.empty()||!enemyEnabled)ImGui::TextWrapped("To arm: choose the Apex profile, load its model and enable incoming-frame analysis.");
   else if(!device.trackingWindowState.valid)ImGui::TextWrapped("To arm: select an Apex window above and restore it if minimized. The list refreshes automatically.");
   ImGui::BeginDisabled(!ready);ImGui::Checkbox("Arm mouse following (hold right mouse to activate)",&trackingOptions.armed);ImGui::EndDisabled();
   hint("Hold right mouse to follow. F9 disarms. F8 only changes overlay visibility.");
   if(!device.trackingEmergency)ImGui::TextWrapped("F9 registration unavailable; mouse following is disabled. Close any app reserving F9 and restart.");
   ImGui::TextWrapped("%s",device.tracking.status.c_str());ImGui::TextWrapped("%s",device.trackingError.c_str());
   section("3. Tune controls");
   if(ImGui::BeginTabBar("TrackingControls")){
    if(ImGui::BeginTabItem("X / horizontal")){hint("Horizontal response. Disabling X stops horizontal corrections.");
ImGui::Checkbox("Enable X",&trackingOptions.x.enabled);ImGui::SliderFloat("X response",&trackingOptions.x.gain,0,5,"%.2f counts / pixel / second");ImGui::SliderFloat("X smoothing",&trackingOptions.x.smoothingMs,0,200,"%.0f ms");ImGui::SliderFloat("X speed limit",&trackingOptions.x.maxSpeed,10,500,"%.0f counts / second");ImGui::EndTabItem();}
    if(ImGui::BeginTabItem("Y / vertical")){hint("Vertical response, independent of X.");
ImGui::Checkbox("Enable Y",&trackingOptions.y.enabled);ImGui::SliderFloat("Y response",&trackingOptions.y.gain,0,5,"%.2f counts / pixel / second");ImGui::SliderFloat("Y smoothing",&trackingOptions.y.smoothingMs,0,200,"%.0f ms");ImGui::SliderFloat("Y speed limit",&trackingOptions.y.maxSpeed,10,500,"%.0f counts / second");ImGui::EndTabItem();}
    if(ImGui::BeginTabItem("FOV overlay")){
bool fovOn=trackingOptions.showFov&&overlayOptions.enabled&&!device.overlayHidden;if(ImGui::Checkbox("Show FOV circle on my screen",&fovOn)){trackingOptions.showFov=fovOn;if(fovOn)overlayOptions.enabled=true;overlayRequest_=1;}ImGui::SliderFloat("FOV radius",&trackingOptions.radius,20,800,"%.0f client pixels");ImGui::TextWrapped("Circle centered on selected game client; this is a pixel radius, not the game's camera FOV angle. Use the same full-view OBS capture and selected overlay monitor.");ImGui::EndTabItem();}
    ImGui::EndTabBar();
   }
   if(ImGui::CollapsingHeader("Tracking limits")){ImGui::SliderFloat("Center deadzone",&trackingOptions.deadzone,0,30,"%.1f px");ImGui::SliderFloat("Box anchor Y",&trackingOptions.anchor,.1f,.9f,"%.2f");ImGui::SliderFloat("Maximum received-frame age",&trackingOptions.maxAgeMs,50,500,"%.0f ms");ImGui::TextWrapped("Anchor is a point inside the model box, not an actual joint. Receiver age excludes unknown OBS sender delay. Slow/failed inference stops mouse output. Adjust game sensitivity and X/Y response manually.");}
   if(ImGui::Button("Save tracking setup")){std::string error;notice_=saveTrackingOptions(Config::defaultPath().parent_path()/"tracking-settings.json",trackingOptions,error)?"Tracking settings saved; arming not saved.":error;}ImGui::SameLine();if(ImGui::Button("Load tracking setup")){std::string error;notice_=loadTrackingOptions(Config::defaultPath().parent_path()/"tracking-settings.json",trackingOptions,error)?"Tracking settings loaded, disarmed.":error;trackingRequest_=2;}
   if(ImGui::CollapsingHeader("Tracking status and diagnostics")){
   ImGui::TextDisabled("Capture %.1f FPS | frame age %.0f ms | %llu accepted Windows events",device.capture.fps,device.tracking.ageMs,static_cast<unsigned long long>(device.trackingSent));
   if(device.tracking.target)ImGui::Text("Candidate track %llu | X error %.1f px | Y error %.1f px",static_cast<unsigned long long>(device.tracking.targetId),device.tracking.errorX,device.tracking.errorY);
   if(ImGui::Button("Copy tracking diagnostics")){auto text="Tracking: "+device.tracking.status+"\nError: "+device.trackingError+"\nWindow: "+trackingWindow.title+"\nFrame age ms: "+std::to_string(device.tracking.ageMs)+"\nCapture FPS: "+std::to_string(device.capture.fps)+"\nBackend: "+device.enemy.latest.backend+"\nModel message: "+device.enemy.message+"\nWindows input events accepted: "+std::to_string(device.trackingSent);ImGui::SetClipboardText(text.c_str());}
   }
   ImGui::TextWrapped("%s",notice_.c_str());if(ImGui::CollapsingHeader("How tracking works"))hint("Only visible model candidates are used. Windows accepting input does not prove Apex uses it. Release right mouse or use F9 to disarm; F8 only changes overlay visibility.");
  }else if(tab_==8){
   section("Actual screen overlay", "This creates a separate, transparent desktop window. It is not the preview image.");
   if(ImGui::Checkbox("Show overlay on my screen",&overlayOptions.enabled))overlayRequest_=1;
   ImGui::TextColored(device.overlayVisible?good:muted,"%s",device.overlayVisible?"VISIBLE ON DESKTOP":device.overlayHidden?"HIDDEN BY F8":"OFF / STARTING");
   if(ImGui::Button("Show on screen",{150,0})){overlayOptions.enabled=true;overlayRequest_=1;}ImGui::SameLine();if(ImGui::Button("Hide overlay",{150,0})){overlayOptions.enabled=false;overlayRequest_=1;}
   hint("When enabled, the APEX VISION badge and configured circle appear on this monitor, even before a model is loaded. F8 toggles visibility; it does not arm tracking.");
   section("Monitor and graphics");
   if(ImGui::BeginCombo("Screen",device.overlayMonitors.empty()?"Primary monitor":device.overlayMonitors[std::clamp(overlayOptions.monitor,0,int(device.overlayMonitors.size())-1)].c_str())){for(int n=0;n<int(device.overlayMonitors.size());++n)if(ImGui::Selectable(device.overlayMonitors[n].c_str(),overlayOptions.monitor==n))overlayOptions.monitor=n;ImGui::EndCombo();}
   ImGui::Checkbox("Draw FOV circle",&trackingOptions.showFov);ImGui::SliderFloat("Circle radius",&trackingOptions.radius,20,800,"%.0f pixels");
   ImGui::Checkbox("Draw detected boxes",&overlayOptions.showBoxes);ImGui::Checkbox("Fit capture aspect ratio",&overlayOptions.fitAspect);
   hint("Without a selected game window, the circle is centered on this monitor. With a valid selected Apex window, it follows that client center. Choose the monitor containing the game.");
   ImGui::TextWrapped("%s",device.overlayStatus.c_str());
   if(ImGui::CollapsingHeader("If you cannot see it"))hint("Use borderless/windowed game mode. Exclusive fullscreen may cover desktop overlays. Check the selected monitor and F8 state. The overlay is click-through and never takes game focus. Detection boxes still require fresh live model results; recordings stay inside the preview.");
   if(ImGui::Button("Open Capture"))tab_=4;ImGui::SameLine();if(ImGui::Button("Open Detection"))tab_=5;
  }else{
   section("Preferences", "Save the renderer, theme and data-lab settings. Model and tracking setup have their own save controls.");
   if(ImGui::Button("Save configuration")){std::string error;notice_=Config::save(Config::defaultPath(),s,error)?"Configuration saved.":error;}
   if(ImGui::Button("Load configuration")){std::string error;notice_=Config::load(Config::defaultPath(),s,error)?"Configuration loaded.":error;}
   if(ImGui::Button("Restore defaults")){s=Settings{};notice_="Defaults restored in memory. Save to persist.";}
   ImGui::TextWrapped("%s",notice_.c_str());
   if(ImGui::CollapsingHeader("File locations")){ImGui::TextWrapped("Configuration: %s",Config::defaultPath().u8string().c_str());ImGui::TextWrapped("Log: %s",Log::path().u8string().c_str());}
  }
  ImGui::PopItemWidth();ImGui::EndChild();ImGui::Separator();
  ImGui::TextColored(trackingOptions.armed?warning:muted,"%s",trackingOptions.armed?"TRACKING ARMED":"TRACKING DISARMED");ImGui::SameLine();ImGui::TextDisabled("  F9 stop  /  F8 overlay  /  Insert menu");
 }
 ImGui::End();ImGui::PopStyleVar(2);s.clamp();
}
}
