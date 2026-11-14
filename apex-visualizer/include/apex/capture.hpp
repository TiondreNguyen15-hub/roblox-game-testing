#pragma once
#include "performance.hpp"
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#ifdef _WIN32
struct IMFMediaSource;
#endif
namespace apex {
struct CapturedFrame{unsigned width=0,height=0;std::uint64_t sequence=0;long long sourceTimestamp100ns=0;double readWaitMs=0;std::vector<unsigned char> rgba;};
struct CaptureSnapshot{bool running=false,paused=false,isRecording=false;std::string state="stopped",message="Capture is off",source;std::uint64_t frames=0;double fps=0,readWaitMs=0;unsigned width=0,height=0;FrameStatistics statistics;std::shared_ptr<const CapturedFrame> frame;};
class CaptureSource {
public:
 ~CaptureSource();CaptureSource()=default;CaptureSource(const CaptureSource&)=delete;CaptureSource& operator=(const CaptureSource&)=delete;
 void start(const std::filesystem::path& file={});void stop();void setPaused(bool paused);CaptureSnapshot snapshot()const;
private:
 void run(std::filesystem::path)noexcept;mutable std::mutex mutex_;std::thread worker_;std::atomic<bool> stop_{false},paused_{false};CaptureSnapshot state_;
#ifdef _WIN32
 IMFMediaSource* source_=nullptr;
#endif
};
}
