#pragma once
#include <chrono>
namespace apex {
class SystemMetrics {
public: bool memoryValid=false,cpuValid=false;unsigned long long workingSet=0,privateBytes=0;void update();double uptime()const;double cpu()const{return cpu_;}
private:
 using Clock=std::chrono::steady_clock;Clock::time_point start_=Clock::now(),sample_=start_;unsigned long long previous_=0;bool initial_=true;double cpu_=0;
};
struct RenderStats {int drawCalls=0,vertices=0,indices=0;double frameMs=0,wallFrameMs=0,processingMs=-1,deliveryMs=-1;};
}
