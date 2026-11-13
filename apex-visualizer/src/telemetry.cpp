#include <algorithm>
#include "apex/telemetry.hpp"
#include "apex/log.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <set>
#include <cmath>
#include <sstream>
namespace apex {
namespace {
float number(const nlohmann::json& j,float low,float high){if(!j.is_number())throw std::runtime_error("Expected numeric field");auto v=j.get<double>();if(!std::isfinite(v)||v<low||v>high)throw std::runtime_error("Numeric field outside bounds");return static_cast<float>(v);}
Vec3 vector(const nlohmann::json& j){if(!j.is_array()||j.size()!=3)throw std::runtime_error("Position/bone must have 3 coordinates");return {number(j[0],-10000,10000),number(j[1],-10000,10000),number(j[2],-10000,10000)};}
}
bool Telemetry::decode(const std::string& text,std::vector<Entity>& output,std::string& error){
 try{
  if(text.size()>262144)throw std::runtime_error("Snapshot exceeds 256 KiB");
  auto j=nlohmann::json::parse(text,[](int depth,nlohmann::json::parse_event_t,nlohmann::json&){if(depth>16)throw std::runtime_error("JSON nesting too deep");return true;});
  if(j.is_object()&&j.contains("players")){
   const auto& items=j.at("players");if(!items.is_array()||items.size()>64)throw std::runtime_error("players must be an array of at most 64 entries");
   std::vector<Entity> next;int index=0;
   for(const auto& item:items){
    Entity e{};e.id=index++;e.team=-1;e.screenSpace=true;e.hasShield=true;
    e.name=item.at("name").get<std::string>();if(e.name.empty()||e.name.size()>64)throw std::runtime_error("Name must be 1-64 bytes");for(unsigned char c:e.name)if(c<32||c==127)throw std::runtime_error("Invalid name");
    e.health=number(item.at("health"),0,100);e.shield=number(item.at("shield"),0,100);e.reportedDistance=number(item.at("distance"),0,100000);
    e.screenX=number(item.at("x"),-100000,100000);e.screenY=number(item.at("y"),-100000,100000);
    if(!item.at("visible").is_boolean())throw std::runtime_error("visible must be boolean");
    e.visible=item.at("visible").get<bool>();
    if(item.contains("width"))e.boxWidth=number(item.at("width"),4,1000);
    if(item.contains("height"))e.boxHeight=number(item.at("height"),8,2000);
    if(item.contains("team")){auto team=number(item.at("team"),0,63);if(team!=std::floor(team))throw std::runtime_error("Fractional team");e.team=static_cast<int>(team);}
    if(item.contains("bones")){const auto& bones=item["bones"];if(!bones.is_array()||bones.size()!=15)throw std::runtime_error("Expected 15 screen-space bones");for(size_t i=0;i<15;++i){if(!bones[i].is_array()||bones[i].size()!=2)throw std::runtime_error("Screen-space bone must be [x,y]");e.screenBones[i]={number(bones[i][0],-100000,100000),number(bones[i][1],-100000,100000)};}e.hasSkeleton=true;}
    next.push_back(e);
   }
   output=std::move(next);error.clear();return true;
  }if(!j.is_object()||j.at("schema")!=1||!j.at("entities").is_array())throw std::runtime_error("Expected schema 1 and entities array");
  const auto& items=j.at("entities");if(items.size()>64)throw std::runtime_error("Maximum 64 entities");
  std::vector<Entity> next;std::set<int> ids;
  for(const auto& item:items){
   Entity e{};e.id=static_cast<int>(number(item.at("id"),0,1000000));if(item.at("id").get<double>()!=e.id||!ids.insert(e.id).second)throw std::runtime_error("Duplicate or fractional ID");
   e.team=static_cast<int>(number(item.at("team"),0,63));if(item.at("team").get<double>()!=e.team)throw std::runtime_error("Fractional team");
   e.health=number(item.at("health"),0,100);if(!item.at("visible").is_boolean())throw std::runtime_error("Visibility must be boolean");
    e.visible=item.at("visible").get<bool>();
   e.name=item.at("name").get<std::string>();if(e.name.empty()||e.name.size()>64)throw std::runtime_error("Name length must be 1-64 bytes");for(unsigned char c:e.name)if(c<32||c==127)throw std::runtime_error("Name contains control characters");
   e.position=vector(item.at("position"));
   if(item.contains("shield")){e.shield=number(item["shield"],0,100);e.hasShield=true;}
   if(item.contains("bones")){e.hasSkeleton=true;if(!item["bones"].is_array()||item["bones"].size()!=15)throw std::runtime_error("Expected 15 bones");for(size_t i=0;i<15;++i)e.bones[i]=vector(item["bones"][i]);}

   next.push_back(e);
  }
  output=std::move(next);error.clear();return true;
 }catch(const std::exception& e){error=e.what();return false;}
}
void Telemetry::poll(const std::filesystem::path& path,double now){
 if(path!=path_){path_=path;entities_.clear();lastGood_=-1;lastPoll_=-1;hasGood_=false;lastText_.clear();lastWrite_={};error_="Waiting for snapshot";}
 if(lastPoll_>=0&&now-lastPoll_<.2)return;
 lastPoll_=now;
 const auto previousError=error_;
 try{
  auto size=std::filesystem::file_size(path);if(size>262144)throw std::runtime_error("Snapshot exceeds 256 KiB");
  // Timestamp is part of freshness: rereading an unchanged file must not make it look live.
  auto write=std::filesystem::last_write_time(path);auto age=std::filesystem::file_time_type::clock::now()-write;
  std::ifstream in(path,std::ios::binary);if(!in)throw std::runtime_error("Cannot open JSON snapshot");std::string text; text.resize(static_cast<size_t>(size));in.read(text.data(),static_cast<std::streamsize>(size));if(in.gcount()!=static_cast<std::streamsize>(size))throw std::runtime_error("Snapshot changed during read");
  if(std::filesystem::last_write_time(path)!=write||std::filesystem::file_size(path)!=size)throw std::runtime_error("Snapshot changed during read");
  if((text==lastText_&&error_.empty())||decode(text,entities_,error_)){if(text!=lastText_||write!=lastWrite_||!hasGood_)lastGood_=now-std::max(0.0,std::chrono::duration<double>(age).count());hasGood_=true;lastText_=text;lastWrite_=write;}
 }catch(const std::exception& e){error_=e.what();}
 if(error_!=previousError)Log::write(error_.empty()?"INFO":"WARN",error_.empty()?"JSON provider recovered":("JSON provider: "+error_));
}
std::string Telemetry::status(double now)const{
 if(!error_.empty())return "FILE ERROR / "+error_+(hasGood_?" / retaining last valid frame":" / no valid snapshot");
 if(!hasGood_)return "WAITING FOR JSON DATA";
 std::ostringstream s;s<<(now-lastGood_>2?"STALE":"LIVE")<<" / "<<entities_.size()<<" entities / age "<<static_cast<int>(std::max(0.,now-lastGood_))<<" s";return s.str();
}
}
