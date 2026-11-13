#pragma once
namespace apex {
struct Settings {
 bool boxes=true, corners=false, skeleton=true, health=true, distance=true, names=true, teams=true, visibility=true;
 bool enemiesOnly=false, visibleOnly=false, vsync=true, animate=true;
 float maxDistance=160.f, menuWidth=710.f, menuHeight=530.f, accent[3]={.63f,.39f,1.f};
 int entityCount=18, frameLimit=144;
 void clamp();
};
}
