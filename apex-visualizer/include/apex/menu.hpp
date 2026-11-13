#pragma once
#include "settings.hpp"
#include "visuals.hpp"
#include "system.hpp"
#include "compute.hpp"
#include <string>
namespace apex {
struct DeviceInfo {std::string gpu;unsigned long long videoMemory=0;bool flipModel=false,gpuTimingValid=false;double gpuMs=0;bool utilizationValid=false,memoryValid=false,processMemoryValid=false;unsigned utilization=0;unsigned long long usedBytes=0,totalBytes=0,processVram=0,vramBudget=0;ComputeSnapshot compute;std::string computeReport;bool processUtilizationValid=false;double processUtilization=0;std::string processCounterStatus;std::string cudaStatus,nvmlStatus,featureLevel;};
enum class MenuPage {Visuals,Rendering,System,Configuration};
class Menu {public:int consumeCompute(){int v=computeRequest_;computeRequest_=0;return v;}void selectPage(MenuPage page){tab_=static_cast<int>(page);}void draw(Settings&,const DeviceInfo&,const SystemMetrics&,const RenderStats&,const VisualStats&,bool shown,float delta,const std::string& sourceStatus="");bool consumeReset(){bool value=reset_;reset_=false;return value;}private:int computeRequest_=0;bool reset_=false;int tab_=0;float fade_=0;std::string notice_;};
}
