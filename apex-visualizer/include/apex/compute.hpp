#pragma once
#include <atomic>
#include <filesystem>
#include <map>
#include <mutex>
#include <thread>
#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include <chrono>
namespace apex {
struct TimingSummary {
 std::uint64_t count=0;double total=0,min=1e30,max=0;
 void add(double ms);double mean()const{return count?total/count:0;}
};
struct ComputeTest {
 std::string state="not_run",message="Not tested",reason="NOT_RUN",blockedBy,profileFile;double measuredWallSeconds=-1;bool verified=false;TimingSummary latency,kernel;
};
struct LibraryDiagnostic {std::string name,path,state,message;unsigned long errorCode=0;};
struct ProcessObservation {double engineBusy=-1,localVramMiB=-1,workingMiB=-1,privateMiB=-1;};
struct ComputeSnapshot {
 bool running=false,finished=false;std::string phase="idle",adapter,ortVersion,providerStatus="Not tested",error;
 int cudaDriverVersion=0;std::string runtimePath;std::vector<std::string> availableProviders;std::vector<LibraryDiagnostic> dependencies;
 ComputeTest cuda,cpu,gpu;std::map<std::string,TimingSummary> renderIntervals;
 ProcessObservation before,after,peak;std::uint64_t observations=0;
 double requestedPhaseSeconds=0;int requestedIterations=0;std::string modelHashDescription="Bundled deterministic MatMul(128x256,256x256)+Relu, float32";
};
struct ComputeOptions {
 std::string adapter;std::uint64_t adapterLuid=0;std::filesystem::path baseDirectory,resultDirectory;int iterations=200;double phaseSeconds=0;
};
// Reference input/output for the bundled synthetic model, shared with portable tests.
std::vector<float> computeInput();std::vector<float> computeWeights();std::vector<float> computeReference();
bool verifyComputeOutput(const float* values,size_t count,std::string& error);
bool computeRunSucceeded(const ComputeSnapshot&,bool requireCuda);
void saveComputeReport(const std::filesystem::path&,const ComputeSnapshot&);
class ComputeRunner {
public:
 ~ComputeRunner();ComputeRunner()=default;ComputeRunner(const ComputeRunner&)=delete;ComputeRunner& operator=(const ComputeRunner&)=delete;
 void start(const ComputeOptions&);void cancel(){cancel_=true;}void stop();ComputeSnapshot snapshot()const;
 void finishCapture();void observe(double frameMs,const ProcessObservation&);
private:
 void run(ComputeOptions)noexcept;void phase(const std::string&);void publish(const ComputeTest&,int which);
 mutable std::mutex mutex_;std::atomic<bool> cancel_{false};std::thread worker_;ComputeSnapshot state_;ProcessObservation latest_;std::ofstream trace_;std::chrono::steady_clock::time_point captureStart_;double lastCapture_=-1;
};
}
