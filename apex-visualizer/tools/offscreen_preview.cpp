// CPU rasterization of the application's actual ImGui vertices for documentation.
#include "apex/menu.hpp"
#include <imgui.h>
#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>
int main(int argc,char** argv){
 const int width=1280,height=800;ImGui::CreateContext();auto& io=ImGui::GetIO();io.IniFilename=nullptr;io.DisplaySize={float(width),float(height)};io.DeltaTime=1.f/60;
 if(std::filesystem::exists("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"))io.Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",16.f);
 unsigned char* atlas;int tw,th;io.Fonts->GetTexDataAsRGBA32(&atlas,&tw,&th);io.Fonts->SetTexID((ImTextureID)1);
 apex::Simulation scene;scene.update(1,24);apex::Settings settings;settings.menuWidth=740;settings.menuHeight=550;apex::Visuals visuals;apex::Menu menu;apex::SystemMetrics system;apex::DeviceInfo device{"CPU documentation preview",0};apex::RenderStats stats;
 ImGui::NewFrame();auto v=visuals.draw(scene,settings,width,height);menu.draw(settings,device,system,stats,v,true,1);ImGui::Render();
 std::vector<unsigned char> image(size_t(width*height*3),0);auto* data=ImGui::GetDrawData();
 auto edge=[](ImVec2 a,ImVec2 b,ImVec2 p){return (p.x-a.x)*(b.y-a.y)-(p.y-a.y)*(b.x-a.x);};
 for(int n=0;n<data->CmdListsCount;++n){const auto* list=data->CmdLists[n];for(const auto& cmd:list->CmdBuffer){if(cmd.UserCallback)continue;
  for(unsigned i=0;i+2<cmd.ElemCount;i+=3){const auto& a=list->VtxBuffer[list->IdxBuffer[cmd.IdxOffset+i]+cmd.VtxOffset];const auto& b=list->VtxBuffer[list->IdxBuffer[cmd.IdxOffset+i+1]+cmd.VtxOffset];const auto& c=list->VtxBuffer[list->IdxBuffer[cmd.IdxOffset+i+2]+cmd.VtxOffset];float area=edge(a.pos,b.pos,c.pos);if(std::abs(area)<.00001f)continue;
   int x0=std::max({0,int(std::floor(std::min({a.pos.x,b.pos.x,c.pos.x}))),int(cmd.ClipRect.x)}),x1=std::min({width,int(std::ceil(std::max({a.pos.x,b.pos.x,c.pos.x}))),int(cmd.ClipRect.z)});
   int y0=std::max({0,int(std::floor(std::min({a.pos.y,b.pos.y,c.pos.y}))),int(cmd.ClipRect.y)}),y1=std::min({height,int(std::ceil(std::max({a.pos.y,b.pos.y,c.pos.y}))),int(cmd.ClipRect.w)});
   for(int y=y0;y<y1;++y)for(int x=x0;x<x1;++x){ImVec2 p{float(x)+.5f,float(y)+.5f};float wa=edge(b.pos,c.pos,p)/area,wb=edge(c.pos,a.pos,p)/area,wc=1-wa-wb;if(wa<0||wb<0||wc<0)continue;
    int tx=std::clamp(int((wa*a.uv.x+wb*b.uv.x+wc*c.uv.x)*tw),0,tw-1),ty=std::clamp(int((wa*a.uv.y+wb*b.uv.y+wc*c.uv.y)*th),0,th-1);auto color=[&](int shift){return wa*((a.col>>shift)&255)+wb*((b.col>>shift)&255)+wc*((c.col>>shift)&255);};
    float alpha=color(24)/255.f*atlas[(ty*tw+tx)*4+3]/255.f;for(int k=0;k<3;++k){float value=color(k*8)*atlas[(ty*tw+tx)*4+k]/255.f;auto& out=image[size_t((y*width+x)*3+k)];out=static_cast<unsigned char>(value*alpha+out*(1-alpha));}
   }
  }
 }}
 std::ofstream out(argc>1?argv[1]:"preview.ppm",std::ios::binary);out<<"P6\n"<<width<<' '<<height<<"\n255\n";out.write(reinterpret_cast<const char*>(image.data()),static_cast<std::streamsize>(image.size()));ImGui::DestroyContext();return out?0:1;
}
