#include "apex/config.hpp"
#include "apex/simulation.hpp"
#include <iostream>
#include <fstream>
#include <limits>
#include <stdexcept>
#define CHECK(x) do{if(!(x))throw std::runtime_error(#x);}while(false)
int main(){try{
 apex::Settings s;s.menuWidth=1;s.entityCount=900;s.accent[0]=std::numeric_limits<float>::quiet_NaN();s.clamp();CHECK(s.menuWidth==560&&s.entityCount==64&&s.accent[0]==.6f);
 auto path=std::filesystem::temp_directory_path()/"apex-visualization-unit.cfg";std::string err;s.corners=true;s.names=false;s.frameLimit=120;
 CHECK(apex::Config::save(path,s,err));apex::Settings read;CHECK(apex::Config::load(path,read,err));CHECK(read.corners&&!read.names&&read.frameLimit==120&&read.entityCount==64);
 {std::ofstream f(path);f<<"version=1\nboxes=2\n";}auto previous=read;CHECK(!apex::Config::load(path,read,err));CHECK(read.corners==previous.corners);
 {std::ofstream f(path);f<<"version=1\nmenuWidth=nan\n";}CHECK(!apex::Config::load(path,read,err));
 {std::ofstream f(path);f<<"version=99\n";}CHECK(!apex::Config::load(path,read,err));
 {std::ofstream f(path);f<<"version=1\nentityCount=4.2\n";}CHECK(!apex::Config::load(path,read,err));
 {std::ofstream f(path);f<<std::string(40000,'x');}CHECK(!apex::Config::load(path,read,err));
 CHECK(apex::Config::save(path,s,err));CHECK(apex::Config::save(path,s,err));CHECK(apex::Config::load(path,read,err));std::filesystem::remove(path);
 apex::Vec2 screen{};CHECK(apex::project({0,1.1f,10},1280,720,screen));CHECK(screen.x==640&&screen.y==360);CHECK(!apex::project({0,0,-1},1280,720,screen));CHECK(!apex::project({0,0,10},0,720,screen));
 apex::Simulation sim;sim.update(0,18);auto initial=sim.entities()[0].position.x;CHECK(sim.entities().size()==18);sim.update(2,18);CHECK(initial!=sim.entities()[0].position.x);
 for(const auto& e:sim.entities()){CHECK(e.health>=0&&e.health<=100);CHECK(e.bones[0].y>e.bones[11].y);CHECK(e.team>=0&&e.team<=2);}
 sim.update(1,1000);CHECK(sim.entities().size()==64);sim.update(1,-2);CHECK(sim.entities().size()==1);
 sim.update(0,5,2);auto fixed=sim.entities()[0].position.x;sim.update(100,5,2);CHECK(sim.entities()[0].position.x==fixed);
 sim.update(0,5,1);auto tracked=sim.entities()[0].position.x;sim.update(2,5,1);CHECK(sim.entities()[0].position.x!=tracked);
 s.scene=2;s.playbackSpeed=2.5f;CHECK(apex::Config::save(path,s,err));CHECK(apex::Config::load(path,read,err));CHECK(read.scene==2&&read.playbackSpeed==2.5f);std::filesystem::remove(path);
 std::cout<<"PASS: settings bounds; config round-trip/atomic overwrite/rejection; projection; simulation invariants\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
