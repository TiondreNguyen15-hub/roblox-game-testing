#pragma once
#include "capture.hpp"
#include <condition_variable>
#include <optional>
namespace apex {
struct EnemyBox{float x1=0,y1=0,x2=0,y2=0,score=0;};
struct EnemyImage{std::vector<float> nchw;float scale=1;unsigned left=0,top=0,width=0,height=0;};
enum class EnemyProfile{Apex416=0,Roblox640=1};
unsigned enemyImageSide(EnemyProfile);
std::filesystem::path enemySettingsFile(const std::filesystem::path&,EnemyProfile);
struct EnemyOptions{EnemyProfile profile=EnemyProfile::Apex416;std::filesystem::path modelPath,baseDirectory,profileDirectory;std::string adapter;std::uint64_t adapterLuid=0;bool preferCuda=true,stabilize=true;float confidence=.5f,iou=.45f;int intervalMs=150,maxResultAgeMs=250,visualGraceMs=120;};
struct EnemyObservation{std::uint64_t sequence=0;long long timestamp100ns=0;double latencyMs=0;std::vector<EnemyBox> boxes;unsigned retained=0;std::string backend,modelSha,modelLabels,source,message;bool cudaVerified=false;};
struct EnemySnapshot{std::chrono::steady_clock::time_point observedAt{};bool busy=false;std::string status="no_model",message="Select your trained enemy ONNX model";std::uint64_t completed=0;EnemyObservation latest;std::shared_ptr<const CapturedFrame> analyzedFrame;};
double enemyFrameAgeMs(const EnemySnapshot&,std::chrono::steady_clock::time_point now=std::chrono::steady_clock::now());
bool enemyResultFresh(const EnemySnapshot&,int maxAgeMs,std::chrono::steady_clock::time_point now=std::chrono::steady_clock::now());
EnemyImage prepareEnemyImage(const CapturedFrame&,unsigned side=416);
std::vector<EnemyBox> decodeEnemyOutput(const float*,size_t,const EnemyImage&,float confidence,float iou);
std::vector<EnemyBox> decodeRobloxOutput(const float*,size_t,const EnemyImage&,float confidence,float iou);
bool saveEnemyOptions(const std::filesystem::path&,const EnemyOptions&,std::string&);
bool loadEnemyOptions(const std::filesystem::path&,EnemyOptions&,std::string&);
class EnemyContinuity{public:void reset();std::vector<EnemyBox> update(const std::vector<EnemyBox>&,const EnemyOptions&,const CapturedFrame&);private:std::vector<EnemyBox> confirmed_;double seen_=-1;unsigned width_=0,height_=0;};
struct EnemyDisplayState{std::vector<EnemyBox> boxes;bool held=false;};
class EnemyDisplayCache{public:void reset();EnemyDisplayState update(const EnemySnapshot&,int maxAgeMs,int graceMs,bool enabled,std::chrono::steady_clock::time_point now=std::chrono::steady_clock::now());private:std::vector<EnemyBox> boxes_;std::chrono::steady_clock::time_point stamp_{},missing_{};std::string source_;unsigned width_=0,height_=0;std::uint64_t completed_=0;};
class EnemyDetector{public:virtual ~EnemyDetector()=default;virtual EnemyObservation run(const CapturedFrame&,const EnemyOptions&)=0;};
std::unique_ptr<EnemyDetector> makeEnemyDetector(const EnemyOptions&);
class EnemyService{
public:~EnemyService();void beginSource();void submit(std::shared_ptr<const CapturedFrame>,const EnemyOptions&,const std::string&,bool force=false);void stop();EnemySnapshot snapshot()const;
private:struct Job{std::shared_ptr<const CapturedFrame> frame;EnemyOptions options;std::string source;std::uint64_t generation=0;};void run()noexcept;mutable std::mutex mutex_;std::condition_variable wake_;std::thread worker_;bool stop_=false;std::optional<Job> pending_;EnemySnapshot state_;std::uint64_t generation_=0,lastSequence_=0;std::chrono::steady_clock::time_point submitted_{};EnemyOptions last_;bool configured_=false;
};
}
