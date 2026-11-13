#pragma once
#include "telemetry.hpp"
#include "settings.hpp"
#include "async_source.hpp"
namespace apex {
// All active providers expose the same frame/status contract. No input source is selected implicitly.
class SimulationSource final:public IDataSource {
public:
 void configure(double time,int count,int scene){time_=time;count_=count;scene_=scene;}
 void poll(const std::filesystem::path&,double)override{simulation_.update(time_,count_,scene_);}
 const std::vector<Entity>& entities()const override{return simulation_.entities();}
 std::string status(double)const override{return "SIMULATION / generated local test data";}
private:Simulation simulation_;double time_=0;int count_=18,scene_=0;
};
class ProviderSystem {
public:
 explicit ProviderSystem(bool background=false):background_(background){}
 void update(const Settings& settings,double now,double simulationTime){
  IDataSource* selected=settings.liveFile?(background_?static_cast<IDataSource*>(&async_):static_cast<IDataSource*>(&file_)):static_cast<IDataSource*>(&simulation_);
  if(selected!=active_){file_.reset();async_.reset();active_=selected;}
  if(settings.liveFile)active_->poll(std::filesystem::u8path(settings.livePath),now);
  else{simulation_.configure(simulationTime,settings.entityCount,settings.scene);active_->poll({},now);}
 }
 const std::vector<Entity>& entities()const {static const std::vector<Entity> empty;return active_?active_->entities():empty;}
 std::string status(double now)const{return active_?active_->status(now):"No provider selected";}
private:bool background_;Telemetry file_;AsyncFileSource async_;SimulationSource simulation_;IDataSource* active_=nullptr;
};
}
