#include "apex/visuals.hpp"
#include "apex/telemetry.hpp"
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
 std::vector<apex::Entity> players;std::string error;
 CHECK(apex::Telemetry::decode(R"({"players":[{"name":"One","health":100,"shield":100,"distance":45,"x":500,"y":300,"visible":true},{"name":"Two","health":75,"shield":50,"distance":90,"x":800,"y":400,"visible":true}]})",players,error));
 settings=apex::Settings{};settings.liveFile=true;ImGui::NewFrame();stats=visuals.draw(players,settings,1280,720);ImGui::Render();CHECK(stats.drawn==2);
 settings.maxDistance=60;ImGui::NewFrame();stats=visuals.draw(players,settings,1280,720);ImGui::Render();CHECK(stats.drawn==1&&stats.filtered==1);
 settings=apex::Settings{};settings.liveFile=true;settings.boxes=false;settings.health=false;settings.shields=false;settings.names=false;settings.distance=false;settings.teams=false;settings.visibility=false;
 ImGui::NewFrame();visuals.draw(players,settings,1280,720);ImGui::Render();int without=ImGui::GetDrawData()->TotalVtxCount;
 settings.skeleton=false;ImGui::NewFrame();visuals.draw(players,settings,1280,720);ImGui::Render();CHECK(ImGui::GetDrawData()->TotalVtxCount==without);
 settings.shields=true;ImGui::NewFrame();visuals.draw(players,settings,1280,720);ImGui::Render();CHECK(ImGui::GetDrawData()->TotalVtxCount>without);
 ImGui::DestroyContext();std::cout<<"PASS: actual ImGui draw generation, entity filters, corner boxes and overlay toggles\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
