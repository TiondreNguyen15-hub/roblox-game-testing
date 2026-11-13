#include "apex/system.hpp"
#include <thread>
#include <algorithm>
#include <ctime>
#ifdef _WIN32
#include <windows.h>
#endif
namespace apex {
#ifdef _WIN32
static unsigned long long ticks(FILETIME t){ULARGE_INTEGER n{};n.LowPart=t.dwLowDateTime;n.HighPart=t.dwHighDateTime;return n.QuadPart;}
#endif
double SystemMetrics::uptime()const{return std::chrono::duration<double>(Clock::now()-start_).count();}
void SystemMetrics::update(){
 auto now=Clock::now();double seconds=std::chrono::duration<double>(now-sample_).count();if(seconds<.5)return;
 unsigned long long total=0;double tickSeconds=0;
#ifdef _WIN32
 FILETIME creation{},exit{},kernel{},user{};
 if(!GetProcessTimes(GetCurrentProcess(),&creation,&exit,&kernel,&user))return;
 total=ticks(kernel)+ticks(user);tickSeconds=1e-7;
#else
 auto ticks=std::clock();if(ticks==static_cast<std::clock_t>(-1))return;
 total=static_cast<unsigned long long>(ticks);tickSeconds=1.0/CLOCKS_PER_SEC;
#endif
 if(!initial_)cpu_=std::clamp((total-previous_)*tickSeconds/seconds/std::max(1u,std::thread::hardware_concurrency())*100.,0.,100.);
 previous_=total;initial_=false;
 sample_=now;
}
}
