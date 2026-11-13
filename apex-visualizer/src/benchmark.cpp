#include "apex/benchmark.hpp"
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <iomanip>
#include <algorithm>
#include <cmath>
namespace apex {
void Benchmark::start(const std::filesystem::path& dir,const std::string& gpu,const std::string& backend,double duration,double warmup){
 if(!std::isfinite(duration)||duration<1||duration>86400||!std::isfinite(warmup)||warmup<0||warmup>300)throw std::runtime_error("Invalid benchmark duration/warmup");
 if(csv_.is_open())throw std::runtime_error("Benchmark already started");count_=0;sum_=max_=0;min_=1e30;firstMemory_=lastMemory_=-1;histogram_.fill(0);finished_=false;
 std::filesystem::create_directories(dir);directory_=dir;gpu_=gpu;backend_=backend;duration_=duration;warmup_=warmup;csv_.open(dir/"frames.csv",std::ios::trunc);if(!csv_)throw std::runtime_error("Cannot open benchmark CSV");
 csv_<<"elapsed_s,frame_ms,cpu_work_ms,process_cpu_percent,gpu_draw_ms,gpu_device_util_percent,device_vram_mib,process_vram_mib,working_set_mib,private_mib,source_processing_ms,source_delivery_ms\n";csv_<<std::fixed<<std::setprecision(4);
}
bool Benchmark::sample(double elapsed,const BenchmarkSample& s){
 if(finished_)return true;
 if(elapsed<warmup_)return false;
 if(elapsed>warmup_+duration_){finish("completed");return true;}
 if(!std::isfinite(s.frameMs)||s.frameMs<=0)return false;
 ++count_;sum_+=s.frameMs;min_=std::min(min_,s.frameMs);max_=std::max(max_,s.frameMs);auto bin=static_cast<size_t>(std::min(10000.,std::floor(s.frameMs*10)));++histogram_[bin];
 if(s.workingSetMiB>=0){if(firstMemory_<0)firstMemory_=s.workingSetMiB;lastMemory_=s.workingSetMiB;}
 csv_<<elapsed<<','<<s.frameMs<<','<<s.cpuWorkMs<<','<<s.processCpu;
 for(double v:{s.gpuMs,s.gpuUtil,s.deviceVramMiB,s.processVramMiB,s.workingSetMiB,s.privateMiB,s.processingMs,s.deliveryMs}){csv_<<',';if(std::isfinite(v)&&v>=0)csv_<<v;}csv_<<'\n';if(!csv_)throw std::runtime_error("Benchmark CSV write failed");return false;
}
void Benchmark::finish(const std::string& outcome){
 if(finished_)return;
 finished_=true;csv_.flush();if(!csv_)throw std::runtime_error("Benchmark CSV flush failed");csv_.close();
 auto quantile=[this](double q){std::uint64_t n=0,target=static_cast<std::uint64_t>(std::ceil(count_*q));for(size_t i=0;i<histogram_.size();++i){n+=histogram_[i];if(n>=target)return i*.1;}return 1000.;};
 nlohmann::json report={{"outcome",outcome},{"gpu",gpu_},{"backend",backend_},{"warmup_seconds",warmup_},{"requested_measurement_seconds",duration_},{"samples",count_},{"quantile_resolution_ms",.1},{"histogram_overflow_ms",1000},{"notes","GPU utilization/VRAM are optional sampled device counters. Empty CSV fields mean unavailable. Frame time includes pacing; GPU timing is latest asynchronous draw duration, not utilization. Memory delta alone does not prove a leak."}};
 report["configuration"]=metadata;
 if(count_){report["average_fps"]=1000./(sum_/count_);report["mean_frame_ms"]=sum_/count_;report["min_frame_ms"]=min_;report["max_frame_ms"]=max_;report["p95_frame_ms_approx"]=quantile(.95);report["p99_frame_ms_approx"]=quantile(.99);}
 if(firstMemory_>=0)report["working_set_delta_mib"]=lastMemory_-firstMemory_;
 std::ofstream out(directory_/"report.json");out<<report.dump(2)<<'\n';out.flush();if(!out)throw std::runtime_error("Benchmark report write failed");
}
}
