#include "apex/benchmark.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <limits>
int main(){auto dir=std::filesystem::temp_directory_path()/"apex-benchmark-test";try{
 std::filesystem::remove_all(dir);apex::Benchmark invalid;bool rejected=false;try{invalid.start(dir,"test","test",-1,0);}catch(...){rejected=true;}if(!rejected)throw std::runtime_error("Invalid duration accepted");
 apex::Benchmark b;b.start(dir,"test adapter","unit test",2,1);apex::BenchmarkSample s;s.frameMs=10;s.workingSetMiB=20;
 b.sample(.5,s);s.frameMs=std::numeric_limits<double>::quiet_NaN();b.sample(1,s);s.frameMs=10;b.sample(1.1,s);s.frameMs=20;s.workingSetMiB=21;b.sample(1.2,s);if(!b.sample(3.1,s))throw std::runtime_error("Timed benchmark failed to finish");
 nlohmann::json j;std::ifstream(dir/"report.json")>>j;if(j["samples"]!=2||j["mean_frame_ms"]!=15||j["working_set_delta_mib"]!=1||j["outcome"]!="completed")throw std::runtime_error("Benchmark aggregation failed");
 std::ifstream csv(dir/"frames.csv");std::string line;std::getline(csv,line);std::getline(csv,line);if(line.find(",,,,,")==std::string::npos)throw std::runtime_error("Unavailable counters not blank");
 csv.close();b.start(dir,"test","empty",1,0);b.finish("interrupted");std::ifstream(dir/"report.json")>>j;if(j.contains("average_fps"))throw std::runtime_error("Invented empty-run FPS");
 std::filesystem::remove_all(dir);std::cout<<"PASS: benchmark validation, warmup, quantiles, unavailable counters, empty/interrupted reports\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';std::error_code ec;std::filesystem::remove_all(dir,ec);return 1;}}
