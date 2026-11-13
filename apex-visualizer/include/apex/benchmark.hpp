#pragma once
#include <filesystem>
#include <fstream>
#include <array>
#include <string>
#include <cstdint>
#include <map>
namespace apex {
struct BenchmarkSample {
 double frameMs=0,cpuWorkMs=0,processCpu=0,gpuMs=-1,gpuUtil=-1,deviceVramMiB=-1,processVramMiB=-1,workingSetMiB=-1,privateMiB=-1,processingMs=-1,deliveryMs=-1;
};
class Benchmark {
public:
 std::map<std::string,std::string> metadata;
 void start(const std::filesystem::path& directory,const std::string& gpu,const std::string& backend,double duration,double warmup);
 bool sample(double elapsed,const BenchmarkSample&);
 void finish(const std::string& outcome);std::uint64_t samples()const{return count_;}
private:
 struct Aggregate{std::uint64_t count=0;double sum=0,min=1e30,max=-1e30;};std::array<Aggregate,10> counters_{};
 std::ofstream csv_;std::filesystem::path directory_;std::string gpu_,backend_;double duration_=0,warmup_=0,sum_=0,max_=0,min_=1e30,firstMemory_=-1,lastMemory_=-1;
 std::uint64_t count_=0;std::array<std::uint64_t,10001> histogram_{};bool finished_=false;
};
}
