#pragma once
#include "settings.hpp"
#include "visuals.hpp"
#include "system.hpp"
#include <string>
namespace apex {
struct DeviceInfo {std::string gpu;unsigned long long videoMemory=0;};
enum class MenuPage {Visuals,Rendering,System,Configuration};
class Menu {public:void selectPage(MenuPage page){tab_=static_cast<int>(page);}void draw(Settings&,const DeviceInfo&,const SystemMetrics&,const RenderStats&,const VisualStats&,bool shown,float delta);private:int tab_=0;float fade_=0;std::string notice_;};
}
