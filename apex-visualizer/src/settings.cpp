#include "apex/settings.hpp"
#include <algorithm>
#include <cmath>
namespace apex {
static float finiteClamp(float v,float lo,float hi,float fallback){return std::isfinite(v)?std::clamp(v,lo,hi):fallback;}
void Settings::clamp(){
 maxDistance=finiteClamp(maxDistance,10,300,160); menuWidth=finiteClamp(menuWidth,560,1200,710);menuHeight=finiteClamp(menuHeight,440,900,530);
 for(auto& v:accent)v=finiteClamp(v,0,1,.6f);
 entityCount=std::clamp(entityCount,1,64);frameLimit=std::clamp(frameLimit,30,360);
}
}
