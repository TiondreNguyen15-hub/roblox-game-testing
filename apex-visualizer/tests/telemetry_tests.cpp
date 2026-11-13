#include "apex/telemetry.hpp"
#include "apex/provider.hpp"
#include <fstream>
#include <algorithm>
#include <thread>
#include <iostream>
#include <stdexcept>
#define CHECK(x) do{if(!(x))throw std::runtime_error(#x);}while(false)
int main(){try{
 const std::string valid=R"({"players":[{"name":"Player 1","health":100,"shield":100,"distance":45,"x":500,"y":300,"visible":true},{"name":"Player 2","health":75,"shield":50,"distance":90,"x":800,"y":400,"visible":true}]})";
 std::vector<apex::Entity> entities;std::string error;CHECK(apex::Telemetry::decode(valid,entities,error));CHECK(entities.size()==2&&entities[1].shield==50&&entities[1].screenX==800&&entities[0].reportedDistance==45&&entities[0].screenSpace);
 CHECK(!apex::Telemetry::decode("{",entities,error));CHECK(entities.size()==2);CHECK(!apex::Telemetry::decode(R"({"players":[{"name":"bad"}]})",entities,error));
 auto invalid=valid;invalid.replace(invalid.find("100"),3,"999");CHECK(!apex::Telemetry::decode(invalid,entities,error));
 CHECK(!apex::Telemetry::decode(std::string(262145,'x'),entities,error));
 CHECK(apex::Telemetry::decode("{\"players\":[]}",entities,error)&&entities.empty());
 auto file=std::filesystem::temp_directory_path()/"apex-file-telemetry-test.json";
 {std::ofstream f(file);f<<valid;}apex::Telemetry source;source.poll(file,0);CHECK(source.entities().size()==2);CHECK(source.status(0).find("LIVE")==0);
 {std::ofstream f(file);f<<"partial";}source.poll(file,.25);CHECK(source.entities().size()==2&&source.status(.25).find("FILE ERROR")==0);
 {std::ofstream f(file);f<<"{\"players\":[]}";}source.poll(file,.5);CHECK(source.entities().empty());
 std::filesystem::last_write_time(file,std::filesystem::file_time_type::clock::now()-std::chrono::seconds(10));source.poll(file,.75);CHECK(source.status(.75).find("STALE")==0);
 std::filesystem::last_write_time(file,std::filesystem::file_time_type::clock::now()-std::chrono::hours(24*365*90));source.poll(file,.99);CHECK(source.status(.99).find("STALE")==0);
 std::filesystem::remove(file);source.poll(file,1.25);CHECK(source.status(1.25).find("FILE ERROR")==0);
 const std::string world=R"({"schema":1,"entities":[{"id":1,"name":"Authorized","team":1,"health":70,"shield":25,"visible":true,"position":[0,0,20]}]})";
 CHECK(apex::Telemetry::decode(world,entities,error));CHECK(!entities[0].hasSkeleton&&!entities[0].synthetic&&entities[0].hasShield);
 std::string screenBones="[";for(int i=0;i<15;++i){if(i)screenBones+=",";screenBones+="[500,300]";}screenBones+="]";
 auto explicitBones=valid;explicitBones.insert(explicitBones.find("\"visible\""),"\"bones\":"+screenBones+",");CHECK(apex::Telemetry::decode(explicitBones,entities,error));CHECK(entities[0].hasSkeleton);
 {std::ofstream f(file);f<<valid;}apex::Settings settings;settings.liveFile=true;auto path=file.u8string();CHECK(path.size()<sizeof(settings.livePath));std::copy(path.begin(),path.end(),settings.livePath);settings.livePath[path.size()]=0;
 apex::ProviderSystem providers;providers.update(settings,0,0);CHECK(providers.entities().size()==2&&!providers.entities()[0].synthetic);
 settings.liveFile=false;providers.update(settings,.3,1);CHECK(providers.entities().size()==18&&providers.entities()[0].synthetic);
 std::filesystem::remove(file);settings.liveFile=true;providers.update(settings,.6,1);CHECK(providers.entities().empty());
 {std::ofstream f(file);f<<valid;}apex::AsyncFileSource worker;worker.poll(file,0);
 for(int i=0;i<100&&worker.entities().empty();++i){std::this_thread::sleep_for(std::chrono::milliseconds(5));worker.poll(file,.01);}
 CHECK(worker.entities().size()==2);CHECK(worker.processingMs>=0&&worker.deliveryMs>=worker.processingMs);worker.reset();CHECK(worker.processingMs<0&&worker.deliveryMs<0);CHECK(worker.entities().empty());std::filesystem::remove(file);
 std::cout<<"PASS: exact players schema, shields/screen data, malformed and bounded input, last-valid retention, refresh, stale and missing files\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
