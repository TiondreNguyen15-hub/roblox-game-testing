#pragma once
#include "capture.hpp"
#include <condition_variable>
#include <deque>
#include <functional>
#include <optional>
namespace apex {
enum class HudMode {Ammo,Result,Text};
struct HudRegion {float x=0,y=0,width=1,height=1;};
struct HudOptions {HudRegion region;HudMode mode=HudMode::Ammo;int scale=2,intervalMs=750;};
struct HudImage {unsigned width=0,height=0;std::vector<unsigned char> rgba;};
struct HudReading {std::uint64_t sequence=0;long long timestamp100ns=0;double latencyMs=0;std::string raw,status="unrecognized",banner,source;std::optional<unsigned> magazine,reserve;HudRegion region;HudMode mode=HudMode::Ammo;};
struct HudSnapshot {bool busy=false,available=false;std::string backend="Not started",status="idle",message="Open a recording, select its HUD region, then enable OCR";std::uint64_t completed=0,replaced=0;HudReading latest;};
bool validHudOptions(const HudOptions&);
HudImage cropHudFrame(const CapturedFrame&,const HudOptions&);
HudReading interpretHudText(const std::string&,HudMode);
bool saveHudOptions(const std::filesystem::path&,const HudOptions&,std::string& error);
bool loadHudOptions(const std::filesystem::path&,HudOptions&,std::string& error);
void writeHudCsv(const std::filesystem::path&,const std::vector<HudReading>&);
class HudTextReader {public:virtual ~HudTextReader()=default;virtual std::string recognize(const HudImage&)=0;virtual std::string description()const=0;};
std::unique_ptr<HudTextReader> makeHudTextReader();
class HudOcrService {
public:
 HudOcrService()=default;~HudOcrService();void beginSource();void submit(std::shared_ptr<const CapturedFrame>,const HudOptions&,const std::string& source,bool force=false);void stop();HudSnapshot snapshot()const;void exportCsv(const std::filesystem::path&)const;
private:
 struct Job{std::shared_ptr<const CapturedFrame> frame;HudOptions options;std::string source;std::uint64_t generation=0;};
 void run()noexcept;mutable std::mutex mutex_;std::condition_variable wake_;std::thread worker_;bool stop_=false;std::optional<Job> pending_;std::uint64_t generation_=0,lastSequence_=0;HudOptions lastOptions_;bool haveOptions_=false;std::chrono::steady_clock::time_point submitted_{};HudSnapshot state_;std::deque<HudReading> history_;
};
}
