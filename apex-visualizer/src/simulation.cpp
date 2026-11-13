#include "apex/simulation.hpp"
#include <cmath>
#include <algorithm>
namespace apex {
float distance(Vec3 p){return std::sqrt(p.x*p.x+p.y*p.y+p.z*p.z);}
bool project(Vec3 p,float w,float h,Vec2& s){
 if(!std::isfinite(p.x)||!std::isfinite(p.y)||!std::isfinite(p.z)||p.z<=.1f||w<=0||h<=0)return false;
 const float focal=h*.5f/std::tan(.5f*1.22173048f);
 s={w*.5f+p.x*focal/p.z,h*.5f-(p.y-1.1f)*focal/p.z};return std::isfinite(s.x)&&std::isfinite(s.y);
}
void Simulation::update(double time,int count){
 count=std::clamp(count,1,64);entities_.resize(static_cast<size_t>(count));
 for(int i=0;i<count;++i){auto& e=entities_[i];float t=static_cast<float>(time);e.id=i;e.team=i%3;e.health=20.f+80.f*(.5f+.5f*std::sin(t*.3f+i));e.visible=std::sin(t*.35f+i*.9f)>-.25f;e.name="Operator "+std::to_string(i+1);
  float x=(i%8-3.5f)*4.2f+std::sin(t*.4f+i)*1.3f,z=14.f+(i/8)*16.f+(i%3)*4.f;e.position={x,0,z};
  const float step=std::sin(t*2.f+i)*.13f;
  const std::array<Vec3,15> pose={{{0,1.8f,0},{0,1.55f,0},{0,1.05f,0},{-.28f,1.48f,0},{-.48f,1.15f,step},{-.58f,.87f,step},{.28f,1.48f,0},{.48f,1.15f,-step},{.58f,.87f,-step},{-.18f,.98f,0},{-.22f,.52f,step},{-.25f,0,step},{.18f,.98f,0},{.22f,.52f,-step},{.25f,0,-step}}};
  for(size_t b=0;b<pose.size();++b)e.bones[b]={x+pose[b].x,pose[b].y,z+pose[b].z};
 }
}
}
