#include "apex/visuals.hpp"
#include <imgui.h>
#include <iostream>
#include <stdexcept>
#define CHECK(x) do{if(!(x))throw std::runtime_error(#x);}while(false)
int main(){try{
 ImGui::CreateContext();auto& io=ImGui::GetIO();io.DisplaySize={1280,720};io.DeltaTime=1.f/60;io.IniFilename=nullptr;unsigned char* pixels;int w,h;io.Fonts->GetTexDataAsRGBA32(&pixels,&w,&h);io.Fonts->SetTexID((ImTextureID)1);
 apex::Simulation simulation;simulation.update(0,24);apex::Settings settings;apex::Visuals visuals;
 ImGui::NewFrame();auto stats=visuals.draw(simulation,settings,1280,720);ImGui::Render();CHECK(stats.submitted==24&&stats.drawn>0);int vertices=ImGui::GetDrawData()->TotalVtxCount;CHECK(vertices>100);
 settings.visibleOnly=true;settings.enemiesOnly=true;ImGui::NewFrame();stats=visuals.draw(simulation,settings,1280,720);ImGui::Render();CHECK(stats.filtered>0&&stats.drawn+stats.filtered==24);
 settings.corners=true;settings.health=false;settings.names=false;settings.distance=false;settings.visibility=false;settings.teams=false;settings.skeleton=false;ImGui::NewFrame();visuals.draw(simulation,settings,1280,720);ImGui::Render();CHECK(ImGui::GetDrawData()->TotalVtxCount>0);
 ImGui::DestroyContext();std::cout<<"PASS: actual ImGui draw generation, entity filters, corner boxes and overlay toggles\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
