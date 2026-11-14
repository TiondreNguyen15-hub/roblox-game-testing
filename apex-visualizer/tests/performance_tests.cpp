#include "apex/performance.hpp"
#include "apex/compute.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace apex;
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
int main(){try{
 std::vector<unsigned char> yuy2={16,128,235,128},rgba;yuy2ToRgba(yuy2,2,1,rgba);require(rgba==std::vector<unsigned char>({0,0,0,255,255,255,255,255}),"YUY2 luma/chroma mapping wrong");
 yuy2={81,90,81,240};yuy2ToRgba(yuy2,2,1,rgba);require(rgba[0]>=250&&rgba[1]<3&&rgba[2]<3&&rgba[3]==255,"YUY2 color conversion wrong");
 bool malformed=false;try{yuy2ToRgba(yuy2,3,1,rgba);}catch(...){malformed=true;}require(malformed,"Odd YUY2 width accepted");
 std::vector<MemorySample> samples;
 for(unsigned i=0;i<=540;++i)samples.push_back({i*.5,512.+std::sin(i*.1),200.});
 require(assessMemoryTrend(samples,true).state=="stable_observed","Stable VRAM falsely flagged");
 for(auto& s:samples)s.vramMiB=512+s.seconds*2;
 require(assessMemoryTrend(samples,true).state=="suspected_growth","Sustained growth missed");
 for(auto& s:samples)s.vramMiB=s.seconds<40?512:712;
 require(assessMemoryTrend(samples,true).state=="stable_observed","One-time caching falsely flagged as continuous leak");
 require(assessMemoryTrend({{0,12,10},{300,400,10}},true).state=="inconclusive","Sparse observations falsely verified");
 for(auto& s:samples)s.vramMiB=-1;
 require(assessMemoryTrend(samples,true).state=="inconclusive","Missing counters falsely verified");
 std::vector<unsigned char> black(32*16*4,0),white(black.size(),255);
 auto first=frameStatistics(black,32,16);auto next=frameStatistics(white,32,16,&black);
 require(!first.changeValid&&first.meanLuma==0,"First frame statistics wrong");
 require(next.changeValid&&std::abs(next.meanAbsoluteChange-1)<1e-6&&std::abs(next.meanLuma-1)<1e-6,"Real pixel change not measured");
 bool rejected=false;try{frameStatistics(white,4097,16);}catch(...){rejected=true;}require(rejected,"Oversized frame accepted");
 ComputeSnapshot s;s.finished=true;s.phase="completed";s.modelBenchmark=true;s.gpuTargetSeconds=300;
 for(auto* r:{&s.cuda,&s.cpu,&s.gpu}){r->verified=true;r->state="passed";r->latency.add(2);}
 s.gpu.measuredWallSeconds=299;require(!computeRunSucceeded(s,true),"Short soak falsely passed");s.gpu.measuredWallSeconds=300;require(computeRunSucceeded(s,true),"Completed execution failed");s.vramTrend.state="suspected_growth";require(!computeRunSucceeded(s,true),"Growth flag ignored");
 std::cout<<"PASS memory trend thresholds, missing/sparse counters, cache growth, real pixel statistics, sustained execution criteria\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
