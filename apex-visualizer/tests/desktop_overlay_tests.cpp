#include "apex/desktop_overlay.hpp"
#include <windows.h>
#include <iostream>
#include <limits>
#include <stdexcept>
void require(bool v,const char* s){if(!v)throw std::runtime_error(s);}
COLORREF pixel(HWND window,int x,int y){RECT r{};GetClientRect(window,&r);HDC screen=GetDC(nullptr),dc=CreateCompatibleDC(screen);HBITMAP bitmap=CreateCompatibleBitmap(screen,r.right,r.bottom);auto old=SelectObject(dc,bitmap);SendMessageW(window,WM_PRINTCLIENT,reinterpret_cast<WPARAM>(dc),PRF_CLIENT);auto p=GetPixel(dc,x,y);SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);ReleaseDC(nullptr,screen);return p;}
int main(){try{
 auto before=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
 {
  apex::DesktopOverlay overlay;apex::DesktopOverlayOptions options;options.enabled=true;options.maxResultAgeMs=1000;
  apex::TrackingOverlay circle;circle.visible=true;circle.useMonitorCenter=true;circle.radius=40;
  auto foreground=GetForegroundWindow();overlay.update(options,{},false,0,0,circle);
  HWND window=FindWindowW(L"ApexVisualizationDesktopOverlay",nullptr);
  require(window&&overlay.visible(),"Enabled overlay did not show on desktop without capture/model");
  require(overlay.status().find("connect live OBS")!=std::string::npos,"Waiting-source status missing");
  auto flags=WS_EX_LAYERED|WS_EX_TRANSPARENT|WS_EX_TOPMOST|WS_EX_NOACTIVATE|WS_EX_TOOLWINDOW;
  require((GetWindowLongPtrW(window,GWL_EXSTYLE)&flags)==flags,"Desktop overlay styles wrong");
  require(GetForegroundWindow()==foreground,"Desktop overlay stole focus");
  require(SendMessageW(window,WM_NCHITTEST,0,0)==HTTRANSPARENT,"Overlay intercepted clicks");
  COLORREF key=1;BYTE alpha=0;DWORD modes=0;require(GetLayeredWindowAttributes(window,&key,&alpha,&modes)&&key==RGB(0,0,0)&&(modes&LWA_COLORKEY),"Transparency missing");
  RECT r{};GetClientRect(window,&r);int cx=r.right/2,cy=r.bottom/2;
  require(pixel(window,cx+39,cy)==RGB(210,110,255),"Desktop FOV circle not painted without capture/focus/model");
  apex::EnemySnapshot snapshot;snapshot.completed=1;snapshot.status="ready";snapshot.observedAt=std::chrono::steady_clock::now();snapshot.analyzedFrame=std::make_shared<apex::CapturedFrame>();snapshot.latest.boxes.push_back({.25f,.25f,.75f,.75f,.9f});
  overlay.update(options,snapshot,true,1920,1080,circle);auto view=apex::overlayViewport(float(r.right),float(r.bottom),1920,1080,true);int x=int(view.x+.25f*view.width),y=int(view.y+.5f*view.height);
  require(pixel(window,x,y)==RGB(210,110,255),"Actual detected box edge not drawn");
  options.showBoxes=false;overlay.update(options,snapshot,true,1920,1080,circle);require(pixel(window,x,y)==RGB(0,0,0),"Box switch did not clear graphics");
  options.showBoxes=true;overlay.update(options,snapshot,true,1920,1080,circle);require(pixel(window,x,y)==RGB(210,110,255),"Box switch did not redraw same fresh result");
  for(int n=0;n<200;++n){++snapshot.completed;snapshot.observedAt=std::chrono::steady_clock::now();overlay.update(options,snapshot,true,1920,1080,circle);}
  auto oldFrame=std::make_shared<apex::CapturedFrame>();oldFrame->capturedAt=std::chrono::steady_clock::now()-std::chrono::seconds(2);snapshot.analyzedFrame=oldFrame;snapshot.observedAt=std::chrono::steady_clock::now();overlay.update(options,snapshot,true,1920,1080,circle);require(pixel(window,x,y)==RGB(0,0,0),"Stale boxes retained");
  overlay.update(options,snapshot,false,0,0,circle);require(overlay.visible(),"On-screen window hid when capture stopped");require(pixel(window,cx+39,cy)==RGB(210,110,255),"Circle disappeared when live source stopped");
  auto invalid=circle;invalid.radius=std::numeric_limits<float>::quiet_NaN();overlay.update(options,{},false,0,0,invalid);require(overlay.visible()&&pixel(window,cx+39,cy)==RGB(0,0,0),"Invalid circle entered painter / hid on-status window");
  options.enabled=false;overlay.update(options,snapshot,true,1920,1080,circle);require(!overlay.visible(),"Off switch did not hide desktop window");
 }
 require(!FindWindowW(L"ApexVisualizationDesktopOverlay",nullptr),"Overlay window leaked");require(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)<=before+4,"GDI handles grew after repaint/cleanup");
 std::cout<<"PASS actual desktop window/FOV without capture/model/focus, topmost/click-through/transparency, independent box switch, source-stop badge/circle, stale clearing, invalid-radius rejection, off switch and 200-repaint cleanup; not an exclusive-fullscreen Apex test\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
