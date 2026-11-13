#pragma once
#include "simulation.hpp"
#include "settings.hpp"
namespace apex {
struct VisualStats {int submitted=0,drawn=0,filtered=0;};
class Visuals {public:VisualStats draw(const std::vector<Entity>&,const Settings&,float width,float height)const;VisualStats draw(const Simulation& s,const Settings& c,float w,float h)const{return draw(s.entities(),c,w,h);}};
}
