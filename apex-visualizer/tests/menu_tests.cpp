#include "apex/menu.hpp"
#include <imgui.h>
#include <iostream>
#include <thread>
#include <stdexcept>
int main(){try{
 ImGui::CreateContext();auto& io=ImGui::GetIO();io.IniFilename=nullptr;io.DisplaySize={1280,720};io.DeltaTime=1.f/60;
 unsigned char* pixels;int w,h;io.Fonts->GetTexDataAsRGBA32(&pixels,&w,&h);io.Fonts->SetTexID((ImTextureID)1);
 apex::Settings settings;apex::DeviceInfo device{"Headless UI test adapter",0};apex::SystemMetrics system;apex::RenderStats stats;apex::VisualStats visuals;apex::Menu menu;
 // START must preserve the user's selected mode and binding in both UI modes.
 for(auto mode:{apex::TrackingActivationMode::Hold,apex::TrackingActivationMode::Toggle}){menu.trackingOptions.activationMode=mode;menu.trackingOptions.activationKey=apex::TrackingActivationKey::Mouse5;menu.trackingOptions.armed=false;menu.startTracking();if(menu.trackingOptions.activationMode!=mode||menu.trackingOptions.activationKey!=apex::TrackingActivationKey::Mouse5||!menu.trackingOptions.armed||menu.consumeTracking()!=3||menu.consumeTracking()!=0)throw std::runtime_error("START changed selected activation mode/binding or lost its runtime action");}menu.trackingOptions.armed=false;menu.trackingOptions.activationMode=apex::TrackingActivationMode::Hold;
 for(auto tab:{apex::MenuPage::Visuals,apex::MenuPage::Rendering,apex::MenuPage::System,apex::MenuPage::Configuration,apex::MenuPage::Capture,apex::MenuPage::Hud,apex::MenuPage::Tracking,apex::MenuPage::Home,apex::MenuPage::Overlay}){
  menu.selectPage(tab);ImGui::NewFrame();menu.draw(settings,device,system,stats,visuals,true,.1f);ImGui::Render();if(ImGui::GetDrawData()->TotalVtxCount==0)throw std::runtime_error("Menu tab produced no draw data");
 }
 // Slow/no model completion must not stop live capture from producing preview draw commands.
 menu.selectPage(apex::MenuPage::GameEsp);auto previewFrame=std::make_shared<apex::CapturedFrame>();previewFrame->width=640;previewFrame->height=360;device.capture.frame=previewFrame;device.captureTexture=77;device.enemyTexture=88;device.enemy.analyzedFrame=device.capture.frame;device.enemy.busy=true;
 auto hasTexture=[](ImTextureID id){auto* data=ImGui::GetDrawData();for(int n=0;n<data->CmdListsCount;++n)for(const auto& cmd:data->CmdLists[n]->CmdBuffer)if(cmd.ElemCount&&cmd.GetTexID()==id)return true;return false;};
 for(bool exact:{false,true}){menu.enemyPreviewAnalyzed=exact;ImGui::NewFrame();menu.draw(settings,device,system,stats,visuals,true,.1f);ImGui::Render();if(!hasTexture(exact?88:77))throw std::runtime_error("Live preview was coupled to model completion / exact preview selection failed");}
 for(int i=0;i<10;++i){ImGui::NewFrame();menu.draw(settings,device,system,stats,visuals,false,.1f);ImGui::Render();}
 std::this_thread::sleep_for(std::chrono::milliseconds(510));system.update();if(system.uptime()<.5||system.cpu()<0||system.cpu()>100)throw std::runtime_error("Invalid process metrics");
 ImGui::DestroyContext();std::cout<<"PASS: nine real ImGui menu pages, live preview independent of busy detector, exact analyzed preview, fade-out frames, process CPU bounds and uptime\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
