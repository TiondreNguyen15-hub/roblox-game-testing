#include "apex/frame_pacer.hpp"
#include "apex/log.hpp"
#include <thread>
#include <algorithm>
namespace apex {
FramePacer::FramePacer(){
 timer_=CreateWaitableTimerExW(nullptr,nullptr,0x00000002,TIMER_MODIFY_STATE|SYNCHRONIZE);
 if(timer_)Log::write("INFO","Frame pacing: high-resolution per-process waitable timer");
 else {timer_=CreateWaitableTimerExW(nullptr,nullptr,0,TIMER_MODIFY_STATE|SYNCHRONIZE);Log::write("WARN",timer_?"Frame pacing: standard waitable timer fallback":"Frame pacing: sleep fallback");}
}
FramePacer::~FramePacer(){if(timer_){CancelWaitableTimer(timer_);CloseHandle(timer_);}}
void FramePacer::wait(std::chrono::steady_clock::time_point deadline){
 auto remaining=deadline-std::chrono::steady_clock::now();if(remaining<=decltype(remaining)::zero())return;
 if(timer_){LARGE_INTEGER due{};due.QuadPart=-std::max<long long>(1,std::chrono::duration_cast<std::chrono::nanoseconds>(remaining).count()/100);
  if(SetWaitableTimerEx(timer_,&due,0,nullptr,nullptr,nullptr,0)&&WaitForSingleObject(timer_,1000)==WAIT_OBJECT_0)return;}
 std::this_thread::sleep_until(deadline);
}
}
