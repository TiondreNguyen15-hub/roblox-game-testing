#include "apex/menu.hpp"
#include "apex/config.hpp"
#include <imgui.h>
#include <algorithm>
#include <cmath>
namespace apex {
void Menu::draw(Settings& s,const DeviceInfo& device,const SystemMetrics& system,const RenderStats& stats,const VisualStats& visuals,bool shown,float dt){
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
  const char* tabs[]={"Visuals","Rendering","System","Configuration"};for(int i=0;i<4;++i)if(ImGui::Selectable(tabs[i],tab_==i,0,{0,36*scale}))tab_=i;
  ImGui::Spacing();ImGui::TextDisabled("INSERT");ImGui::TextWrapped("Toggle menu\nF10: exit");ImGui::EndChild();ImGui::SameLine();
  ImGui::BeginChild("Page",{0,-28},false);
  ImGui::Text("%s",tabs[tab_]);ImGui::Separator();
  if(tab_==0){
   ImGui::TextDisabled("All entities below are generated inside this application.");
   ImGui::Checkbox("2D bounding boxes",&s.boxes);ImGui::SameLine();ImGui::Checkbox("Corner style",&s.corners);
   ImGui::Checkbox("Skeleton",&s.skeleton);ImGui::SameLine();ImGui::Checkbox("Health bars",&s.health);
   ImGui::Checkbox("Names",&s.names);ImGui::SameLine();ImGui::Checkbox("Distances",&s.distance);
   ImGui::Checkbox("Team identifiers",&s.teams);ImGui::SameLine();ImGui::Checkbox("Visibility labels",&s.visibility);
   ImGui::Checkbox("Enemies only (hide team 1)",&s.enemiesOnly);ImGui::Checkbox("Visible only",&s.visibleOnly);
   ImGui::SliderFloat("Max distance",&s.maxDistance,10,300,"%.0f m");ImGui::SliderInt("Simulated entities",&s.entityCount,1,64);
  }else if(tab_==1){
   ImGui::Checkbox("VSync",&s.vsync);ImGui::Checkbox("Animate simulated scene",&s.animate);ImGui::SliderInt("Frame cap",&s.frameLimit,30,360);
   ImGui::SliderFloat("Menu width",&s.menuWidth,560,1200,"%.0f px");ImGui::SliderFloat("Menu height",&s.menuHeight,440,900,"%.0f px");ImGui::ColorEdit3("Purple accent",s.accent);
   ImGui::TextWrapped("VSync uses the monitor refresh rate. The frame cap also limits CPU usage. The scene pauses while minimized or occluded.");
  }else if(tab_==2){
   ImGui::Text("FPS: %.1f",ImGui::GetIO().Framerate);ImGui::Text("Previous frame CPU work: %.3f ms",stats.frameMs);ImGui::Text("Process CPU: %.2f%% (all logical cores)",system.cpu());
   ImGui::TextWrapped("GPU: %s",device.gpu.c_str());ImGui::Text("Dedicated video memory: %.0f MiB",double(device.videoMemory)/1048576.);ImGui::Text("Application uptime: %.1f s",system.uptime());
   ImGui::Separator();ImGui::Text("Previous frame draw calls: %d",stats.drawCalls);ImGui::Text("Vertices: %d / indices: %d",stats.vertices,stats.indices);ImGui::Text("Entities: %d drawn / %d filtered",visuals.drawn,visuals.filtered);
  }else{
   ImGui::TextWrapped("Configuration: %s",Config::defaultPath().u8string().c_str());
   if(ImGui::Button("Save configuration")){std::string error;notice_=Config::save(Config::defaultPath(),s,error)?"Configuration saved.":error;}
   if(ImGui::Button("Load configuration")){std::string error;notice_=Config::load(Config::defaultPath(),s,error)?"Configuration loaded.":error;}
   if(ImGui::Button("Restore defaults")){s=Settings{};notice_="Defaults restored in memory. Save to persist.";}
   ImGui::TextWrapped("%s",notice_.c_str());
  }
  ImGui::EndChild();ImGui::Separator();ImGui::TextDisabled("Simulated data | DX11 | ImGui | Insert: menu | F10: exit");
 }
 ImGui::End();ImGui::PopStyleVar();s.clamp();
}
}
