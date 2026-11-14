#pragma once
#include <string>
#include <vector>
namespace apex {
struct MemorySample{double seconds=0,vramMiB=-1,privateMiB=-1;};
struct MemoryTrend{std::string state="inconclusive",message="Not enough sustained samples";double growthMiB=0,slopeMiBPerMinute=0,spanSeconds=0;unsigned samples=0;};
MemoryTrend assessMemoryTrend(const std::vector<MemorySample>& samples,bool vram);
struct FrameStatistics{double meanLuma=0,meanAbsoluteChange=0;bool changeValid=false;};
void yuy2ToRgba(const std::vector<unsigned char>& input,unsigned width,unsigned height,std::vector<unsigned char>& output);
FrameStatistics frameStatistics(const std::vector<unsigned char>& rgba,unsigned width,unsigned height,const std::vector<unsigned char>* previous=nullptr);
}
