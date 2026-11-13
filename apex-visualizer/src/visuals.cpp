#include "apex/visuals.hpp"
#include <imgui.h>
#include <cstdio>
#include <algorithm>
namespace apex {
VisualStats Visuals::draw(const std::vector<Entity>& entities,const Settings& cfg,float w,float h)const{
 auto* d=ImGui::GetBackgroundDrawList();VisualStats stats{};
 d->AddRectFilledMultiColor({0,0},{w,h},IM_COL32(15,17,25,255),IM_COL32(25,21,38,255),IM_COL32(9,14,24,255),IM_COL32(12,16,23,255));
 d->PushClipRect({0,0},{w,h},true);
 for(int z=5;z<=140;z+=5){Vec2 a,b;if(project({-100,0,float(z)},w,h,a)&&project({100,0,float(z)},w,h,b))d->AddLine({a.x,a.y},{b.x,b.y},IM_COL32(60,67,86,90));}
 for(int x=-80;x<=80;x+=5){Vec2 a,b;if(project({float(x),0,4},w,h,a)&&project({float(x),0,140},w,h,b))d->AddLine({a.x,a.y},{b.x,b.y},IM_COL32(60,67,86,90));}
 d->AddText({22,18},IM_COL32(180,157,230,255),cfg.liveFile?"APEX VISUALIZATION / AUTHORIZED LOCAL JSON":"APEX VISUALIZATION / SIMULATED ENTITIES ONLY");
 static constexpr int links[][2]={{0,1},{1,2},{1,3},{3,4},{4,5},{1,6},{6,7},{7,8},{2,9},{9,10},{10,11},{2,12},{12,13},{13,14}};
 for(const auto& e:entities){
  ++stats.submitted;float range=e.reportedDistance>=0?e.reportedDistance:distance(e.position);Vec2 head,feet;
  if((cfg.enemiesOnly&&e.team==0)||(cfg.visibleOnly&&!e.visible)||range>cfg.maxDistance||(!e.screenSpace&&(!project({e.position.x,e.position.y+1.8f,e.position.z},w,h,head)||!project(e.position,w,h,feet)))){++stats.filtered;continue;}
  float height=e.screenSpace?e.boxHeight:feet.y-head.y+6,width=e.screenSpace?e.boxWidth:height*.46f;float x=e.screenSpace?e.screenX:head.x-width*.5f,y=e.screenSpace?e.screenY:head.y-3;
  if(e.screenSpace){head={x+width*.5f,y+3};feet={head.x,y+height};}
  if(height<3||x+width<0||x>w||y+height<0||y>h){++stats.filtered;continue;}
  ++stats.drawn;
  ImU32 color=e.visible?IM_COL32(176,124,255,255):IM_COL32(113,119,140,255);
  // Neutral silhouettes make the simulated scene legible even when overlays are disabled.
  if(e.synthetic){d->AddLine({head.x,head.y+height*.2f},{feet.x,feet.y-height*.16f},IM_COL32(64,72,92,200),width*.34f);
  d->AddCircleFilled({head.x,head.y+height*.07f},std::max(2.f,height*.065f),IM_COL32(93,101,121,255),10);}
  if(cfg.boxes){
   if(cfg.corners){float a=width*.28f,b=height*.2f;for(int side=0;side<2;++side)for(int bottom=0;bottom<2;++bottom){float px=x+side*width,py=y+bottom*height;d->AddLine({px,py},{px+(side?-a:a),py},color,1.5f);d->AddLine({px,py},{px,py+(bottom?-b:b)},color,1.5f);}}
   else d->AddRect({x,y},{x+width,y+height},color,0,0,1.3f);
  }
  if(cfg.skeleton&&e.hasSkeleton)for(const auto& link:links){Vec2 a,b;bool valid=true;if(e.screenSpace){a=e.screenBones[link[0]];b=e.screenBones[link[1]];}else valid=project(e.bones[link[0]],w,h,a)&&project(e.bones[link[1]],w,h,b);if(valid)d->AddLine({a.x,a.y},{b.x,b.y},IM_COL32(224,219,242,235),1.f);}
  if(cfg.health){float fraction=std::clamp(e.health/100.f,0.f,1.f);d->AddRectFilled({x-7,y},{x-3,y+height},IM_COL32(30,32,40,255));d->AddRectFilled({x-6,y+height*(1-fraction)},{x-4,y+height},IM_COL32(int(255*(1-fraction)),int(220*fraction),80,255));}
  float bottom=y+height+3;
  if(cfg.shields&&e.hasShield){float fraction=std::clamp(e.shield/100.f,0.f,1.f);d->AddRectFilled({x,bottom},{x+width,bottom+4},IM_COL32(30,40,65,255));d->AddRectFilled({x,bottom},{x+width*fraction,bottom+4},IM_COL32(80,160,255,255));bottom+=7;}
  char values[64];
  if(cfg.health|| (cfg.shields&&e.hasShield)){
   if(cfg.health&&cfg.shields&&e.hasShield)std::snprintf(values,sizeof(values),"HP %.0f / SH %.0f",e.health,e.shield);
   else if(cfg.health)std::snprintf(values,sizeof(values),"HP %.0f",e.health);
   else std::snprintf(values,sizeof(values),"SH %.0f",e.shield);
   d->AddText({x,bottom},IM_COL32(190,210,240,255),values);bottom+=15;
  }
  float ty=y-16;
  if(cfg.names){auto size=ImGui::CalcTextSize(e.name.c_str());d->AddText({head.x-size.x*.5f,ty},IM_COL32(234,233,243,255),e.name.c_str());ty-=15;}
  char buf[96];
  const bool showTeam=cfg.teams&&e.team>=0;
  if(showTeam||cfg.visibility){std::snprintf(buf,sizeof(buf),"%s%d %s",showTeam?"T":"",showTeam?e.team+1:0,cfg.visibility?(e.visible?"VISIBLE":"OCCLUDED"):"");if(!showTeam)std::snprintf(buf,sizeof(buf),"%s",e.visible?"VISIBLE":"OCCLUDED");d->AddText({x,ty},color,buf);}
  if(cfg.distance){std::snprintf(buf,sizeof(buf),"%.0f m",range);d->AddText({x,bottom},IM_COL32(185,192,207,255),buf);}
 }
 d->PopClipRect();return stats;
}
}
