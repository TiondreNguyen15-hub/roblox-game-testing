#include "apex/telemetry.hpp"
#include <fstream>
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
 std::filesystem::remove(file);source.poll(file,1);CHECK(source.status(1).find("FILE ERROR")==0);
 std::cout<<"PASS: exact players schema, shields/screen data, malformed and bounded input, last-valid retention, refresh, stale and missing files\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
