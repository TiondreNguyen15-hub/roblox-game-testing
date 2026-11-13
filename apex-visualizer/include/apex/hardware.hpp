#pragma once
#include <windows.h>
#include <string>
#include <pdh.h>
#include <pdhmsg.h>
namespace apex {
// All counters below are optional. Missing driver APIs never become zero-success values.
class HardwareMetrics {
public:
 HardwareMetrics()=default;~HardwareMetrics();HardwareMetrics(const HardwareMetrics&)=delete;HardwareMetrics& operator=(const HardwareMetrics&)=delete;
 void initialize(const std::string& adapter,LUID luid={});void update(double now);
 bool processUtilizationValid=false;double processUtilization=0;std::string processCounterStatus="Not sampled";
 bool utilizationValid=false,memoryValid=false;unsigned utilization=0;unsigned long long usedBytes=0,totalBytes=0;
 std::string nvmlStatus="Not probed",cudaStatus="Not probed";bool cudaAvailable=false;int cudaDriver=0,computeMajor=0,computeMinor=0;
private:
 PDH_HQUERY processQuery_=nullptr;PDH_HCOUNTER processCounter_=nullptr;std::wstring pidMatch_,luidMatch_;bool pdhPrimed_=false;
 HMODULE nvml_=nullptr;void* device_=nullptr;bool initialized_=false;double sampled_=-1;
 using UtilFn=int(__cdecl*)(void*,void*);using MemoryFn=int(__cdecl*)(void*,void*);using ShutdownFn=int(__cdecl*)();
 UtilFn utilizationFn_=nullptr;MemoryFn memoryFn_=nullptr;ShutdownFn shutdown_=nullptr;
};
}
