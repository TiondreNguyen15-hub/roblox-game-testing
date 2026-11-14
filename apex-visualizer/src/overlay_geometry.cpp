#include "apex/desktop_overlay.hpp"
#include <algorithm>
#include <cmath>
namespace apex {
OverlayViewport overlayViewport(float w,float h,unsigned sw,unsigned sh,bool fit){
 if(!std::isfinite(w)||!std::isfinite(h)||w<=0||h<=0||!sw||!sh)return {};
 if(!fit)return {0,0,w,h};float scale=std::min(w/sw,h/sh),width=sw*scale,height=sh*scale;return {(w-width)*.5f,(h-height)*.5f,width,height};
}
}
