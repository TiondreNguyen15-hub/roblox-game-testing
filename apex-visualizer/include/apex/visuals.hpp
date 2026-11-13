#pragma once
#include "simulation.hpp"
#include "settings.hpp"
namespace apex {
struct VisualStats {int submitted=0,drawn=0,filtered=0;};
class Visuals {public:VisualStats draw(const Simulation&,const Settings&,float width,float height)const;};
}
