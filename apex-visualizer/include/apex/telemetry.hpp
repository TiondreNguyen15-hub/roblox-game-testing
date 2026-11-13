#pragma once
#include "simulation.hpp"
#include <filesystem>
#include <string>
namespace apex {
class IDataSource {public:virtual ~IDataSource()=default;virtual void poll(const std::filesystem::path&,double)=0;virtual const std::vector<Entity>& entities()const=0;virtual std::string status(double)const=0;};
class Telemetry final:public IDataSource {
public:
 void reset(){*this=Telemetry{};}
 void poll(const std::filesystem::path& path,double now);
 const std::vector<Entity>& entities()const{return entities_;}
 std::string status(double now)const;
 static bool decode(const std::string&,std::vector<Entity>&,std::string&);
private:
 std::vector<Entity> entities_;std::filesystem::path path_;double lastPoll_=-1,lastGood_=-1;
 bool hasGood_=false;std::string lastText_;std::filesystem::file_time_type lastWrite_{};
 std::string error_="No snapshot received";
};
}
