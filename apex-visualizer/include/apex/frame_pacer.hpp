#pragma once
#include <windows.h>
#include <chrono>
namespace apex {
// Per-process high-resolution timer; never changes system-wide timer resolution.
class FramePacer {
public:
 FramePacer();~FramePacer();FramePacer(const FramePacer&)=delete;FramePacer& operator=(const FramePacer&)=delete;
 void wait(std::chrono::steady_clock::time_point deadline);
private:HANDLE timer_=nullptr;
};
}
