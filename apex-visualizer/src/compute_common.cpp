#include "apex/compute.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <stdexcept>
namespace apex {
void TimingSummary::add(double ms){if(!std::isfinite(ms)||ms<0)throw std::runtime_error("Invalid measured duration");++count;total+=ms;min=std::min(min,ms);max=std::max(max,ms);}
std::vector<float> computeInput(){std::vector<float> v(128*256);for(size_t i=0;i<v.size();++i)v[i]=float(int(i%17)-8)*.0625f;return v;}
std::vector<float> computeWeights(){std::vector<float> v(256*256);for(size_t i=0;i<v.size();++i)v[i]=float(int(i%13)-6)*.03125f;return v;}
std::vector<float> computeReference(){auto a=computeInput(),b=computeWeights();std::vector<float> out(128*256);for(size_t m=0;m<128;++m)for(size_t n=0;n<256;++n){float sum=0;for(size_t k=0;k<256;++k)sum+=a[m*256+k]*b[k*256+n];out[m*256+n]=std::max(0.f,sum);}return out;}
bool verifyComputeOutput(const float* values,size_t count,std::string& error){
 static const auto reference=computeReference();if(!values||count!=reference.size()){error="Wrong inference output shape/count";return false;}
 for(size_t i=0;i<count;++i)if(!std::isfinite(values[i])||std::abs(values[i]-reference[i])>1e-4f+1e-3f*std::abs(reference[i])){error="Inference numerical validation failed at index "+std::to_string(i);return false;}
 error.clear();return true;
}
void saveComputeReport(const std::filesystem::path& directory,const ComputeSnapshot& s){
 using J=nlohmann::json;auto timing=[](const TimingSummary& t)->J{if(!t.count)return nullptr;return {{"observations",t.count},{"mean_ms",t.mean()},{"min_ms",t.min},{"max_ms",t.max},{"serial_latency_equivalent_per_second",t.total>0?1000./t.mean():0.}};};
 auto test=[&](const ComputeTest& t)->J{return {{"state",t.state},{"verified",t.verified},{"message",t.message},{"run_end_to_end",timing(t.latency)},{"cuda_kernel_events",timing(t.kernel)}};};
 auto memory=[](const ProcessObservation& p)->J{J j;for(const auto& v:std::initializer_list<std::pair<const char*,double>>{{"process_busiest_engine_percent",p.engineBusy},{"process_local_vram_mib",p.localVramMiB},{"working_set_mib",p.workingMiB},{"private_mib",p.privateMiB}}){if(v.second>=0&&std::isfinite(v.second))j[v.first]=v.second;else j[v.first]=nullptr;}return j;};
 J j={{"finished",s.finished},{"phase",s.phase},{"adapter",s.adapter},{"onnx_runtime_version",s.ortVersion},{"acceleration_status",s.providerStatus},{"error",s.error},{"model",s.modelHashDescription},{"requested_iterations_per_phase",s.requestedIterations},{"minimum_seconds_per_available_phase",s.requestedPhaseSeconds},{"cuda_workload",test(s.cuda)},{"cpu_inference",test(s.cpu)},{"cuda_ep_inference",test(s.gpu)},{"process_before",memory(s.before)},{"process_after_cleanup",memory(s.after)},{"process_peak_observed",memory(s.peak)},{"process_observations",s.observations}};
 for(const auto& p:s.renderIntervals)j["observed_render_frame_intervals"][p.first]=timing(p.second);
 if(s.cpu.verified&&s.gpu.verified&&s.cpu.latency.count&&s.gpu.latency.count&&s.gpu.latency.mean()>0)j["cpu_to_cuda_ep_latency_ratio"]=s.cpu.latency.mean()/s.gpu.latency.mean();else j["cpu_to_cuda_ep_latency_ratio"]=nullptr;
 j["measurement_notes"]="Synthetic authorized compute only. ONNX Run wall latency includes transfers, execution and output materialization; output validation and setup excluded. Serial inference/s is reciprocal mean Run latency, not display FPS. CUDA event duration measures the vector-add kernel, not inference. CUDA EP marked verified only with numeric agreement, disabled CPU fallback and a CUDAExecutionProvider node in actual ORT profiling. Render FPS is reciprocal of observed main-window frame intervals during each phase; short phases can have too few frames to compare. Process GPU busy is PDH busiest matching PID+adapter engine; local VRAM is per-process DXGI WDDM usage. Counters sampled at 2Hz can miss short peaks. Memory endpoints/ranges do not establish leak freedom.";
 std::filesystem::create_directories(directory);std::ofstream out(directory/"compute-report.json");out<<j.dump(2)<<'\n';out.flush();if(!out)throw std::runtime_error("Compute report write failed");
}
}
