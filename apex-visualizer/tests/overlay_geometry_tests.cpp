#include "apex/desktop_overlay.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
void require(bool v){if(!v)throw std::runtime_error("Overlay mapping failed");}
int main(){try{auto v=apex::overlayViewport(1920,1080,1920,1080,true);require(v.x==0&&v.y==0&&v.width==1920&&v.height==1080);v=apex::overlayViewport(2560,1440,640,480,true);require(v.x==320&&v.y==0&&v.width==1920&&v.height==1440);v=apex::overlayViewport(1920,1080,2560,1080,true);require(v.y==135&&v.width==1920&&v.height==810);v=apex::overlayViewport(1920,1080,640,480,false);require(v.x==0&&v.width==1920&&v.height==1080);require(apex::overlayViewport(0,100,1,1,true).width==0&&apex::overlayViewport(100,100,0,0,true).width==0);std::cout<<"PASS overlay fit/stretch, pillarbox/letterbox and invalid dimensions\n";return 0;}catch(const std::exception& e){std::cerr<<e.what();return 1;}}
