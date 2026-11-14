#pragma once
#include "enemy.hpp"
namespace apex {
struct DesktopOverlayOptions {bool enabled=false,fitAspect=true;int monitor=0;};
struct OverlayViewport {float x=0,y=0,width=0,height=0;};
OverlayViewport overlayViewport(float width,float height,unsigned sourceWidth,unsigned sourceHeight,bool fit);
#ifdef _WIN32
class DesktopOverlay {
public:DesktopOverlay();~DesktopOverlay();DesktopOverlay(const DesktopOverlay&)=delete;DesktopOverlay& operator=(const DesktopOverlay&)=delete;
 void update(const DesktopOverlayOptions&,const EnemySnapshot&,bool liveCapture,unsigned width,unsigned height);
 const std::vector<std::string>& monitors()const;const std::string& status()const;
private:struct Impl;std::unique_ptr<Impl> impl_;
};
#endif
}
