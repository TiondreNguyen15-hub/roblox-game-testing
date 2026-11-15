#pragma once
#include "enemy.hpp"
namespace apex {
struct TrackingAxis {bool enabled=true;float gain=1.2f,smoothingMs=60,maxSpeed=160;};
struct TrackingOptions {bool armed=false,showFov=true;TrackingAxis x,y;float radius=150,deadzone=3,anchor=.5f,maxAgeMs=250;};
struct TrackingContext {double now=0,frameTime=0;std::uint64_t sequence=0;float width=0,height=0;bool live=false,focused=false,hold=false,emergencyAvailable=false;};
struct TrackingResult {bool target=false,engaged=false;float x=0,y=0,errorX=0,errorY=0,ageMs=0;int moveX=0,moveY=0;std::uint64_t targetId=0;std::string status="Disarmed";};
struct TrackingOverlay {bool visible=false,marker=false;float centerX=0,centerY=0,radius=150,targetX=0,targetY=0;std::string label;};
struct TrackingWindow {std::uintptr_t handle=0;unsigned long process=0;std::string title;};
struct TrackingWindowState {bool valid=false,focused=false,hold=false;float left=0,top=0,width=0,height=0;std::string status;};
class TargetTracker {
public:TrackingResult update(const std::vector<EnemyBox>&,const TrackingContext&,const TrackingOptions&);void reset();
private:bool locked_=false;EnemyBox box_;std::uint64_t identity_=0,nextIdentity_=1,lastSequence_=0;double lastFrame_=-1,lastTick_=-1;float velocityX_=0,velocityY_=0,remainderX_=0,remainderY_=0;
};
bool saveTrackingOptions(const std::filesystem::path&,const TrackingOptions&,std::string&);
bool loadTrackingOptions(const std::filesystem::path&,TrackingOptions&,std::string&);
#ifdef _WIN32
std::vector<TrackingWindow> trackingWindows();
TrackingWindowState inspectTrackingWindow(const TrackingWindow&);
bool sendTrackingMovement(const TrackingWindow&,int x,int y,std::string& error);
#endif
}
