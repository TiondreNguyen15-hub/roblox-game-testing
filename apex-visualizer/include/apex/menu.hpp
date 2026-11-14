#pragma once
#include "settings.hpp"
#include "visuals.hpp"
#include "system.hpp"
#include "compute.hpp"
#include "capture.hpp"
#include "hud.hpp"
#include "enemy.hpp"
#include <string>
#include <algorithm>
#include <stdexcept>
namespace apex {
struct DeviceInfo {std::string gpu;unsigned long long videoMemory=0;bool flipModel=false,gpuTimingValid=false;double gpuMs=0;bool utilizationValid=false,memoryValid=false,processMemoryValid=false;unsigned utilization=0;unsigned long long usedBytes=0,totalBytes=0,processVram=0,vramBudget=0;EnemySnapshot enemy;std::uint64_t enemyTexture=0;HudSnapshot hud;CaptureSnapshot capture;std::uint64_t captureTexture=0;ComputeSnapshot compute;std::string computeReport;bool processUtilizationValid=false;double processUtilization=0;std::string processCounterStatus;std::string cudaStatus,nvmlStatus,featureLevel;};
enum class MenuPage {Visuals,Rendering,System,Configuration,Capture,GameEsp,Hud=GameEsp};
class Menu {public:bool showTextTools=false;EnemyOptions enemyOptions;bool enemyEnabled=false;int consumeEnemy(){int v=enemyRequest_;enemyRequest_=0;return v;}HudOptions hudOptions;bool hudEnabled=false;int consumeHud(){int v=hudRequest_;hudRequest_=0;return v;}void setRecordingPath(const std::string& path){if(path.size()>=sizeof(recordingPath_))throw std::runtime_error("Recording path is too long for this field");std::copy(path.begin(),path.end(),recordingPath_);recordingPath_[path.size()]=0;}const char* modelPath()const{return modelPath_;}const char* inputProfile()const{return inputProfile_;}const char* recordingPath()const{return recordingPath_;}int consumeCapture(){int v=captureRequest_;captureRequest_=0;return v;}int consumeCompute(){int v=computeRequest_;computeRequest_=0;return v;}void selectPage(MenuPage page){tab_=static_cast<int>(page);}void draw(Settings&,const DeviceInfo&,const SystemMetrics&,const RenderStats&,const VisualStats&,bool shown,float delta,const std::string& sourceStatus="");bool consumeReset(){bool value=reset_;reset_=false;return value;}private:int enemyRequest_=0;int hudRequest_=0;bool hudDragging_=false;float hudAnchorX_=0,hudAnchorY_=0;char modelPath_[1024]{},inputProfile_[1024]{},recordingPath_[1024]{};int captureRequest_=0;int computeRequest_=0;bool reset_=false;int tab_=0;float fade_=0;std::string notice_;};
}
