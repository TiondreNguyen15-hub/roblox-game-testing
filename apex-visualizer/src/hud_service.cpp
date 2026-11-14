#include "apex/hud.hpp"
#include "apex/log.hpp"
namespace apex {namespace {
bool sameOptions(const HudOptions& a,const HudOptions& b){return a.mode==b.mode&&a.scale==b.scale&&a.region.x==b.region.x&&a.region.y==b.region.y&&a.region.width==b.region.width&&a.region.height==b.region.height;}
}
HudOcrService::~HudOcrService(){stop();}
void HudOcrService::stop(){{std::lock_guard<std::mutex> lock(mutex_);stop_=true;pending_.reset();}wake_.notify_all();if(worker_.joinable())worker_.join();}
void HudOcrService::beginSource(){std::lock_guard<std::mutex> lock(mutex_);++generation_;pending_.reset();lastSequence_=0;haveOptions_=false;history_.clear();state_.latest=HudReading{};state_.completed=0;state_.status="idle";state_.message="Select a region in the current recording";}
HudSnapshot HudOcrService::snapshot()const{std::lock_guard<std::mutex> lock(mutex_);return state_;}
void HudOcrService::exportCsv(const std::filesystem::path& path)const{std::vector<HudReading> rows;{std::lock_guard<std::mutex> lock(mutex_);rows.assign(history_.begin(),history_.end());}writeHudCsv(path,rows);}
void HudOcrService::submit(std::shared_ptr<const CapturedFrame> f,const HudOptions& o,const std::string& source,bool force){
 if(!f)return;if(!validHudOptions(o))throw std::runtime_error("Invalid HUD options");std::lock_guard<std::mutex> lock(mutex_);auto now=std::chrono::steady_clock::now();bool changed=!haveOptions_||!sameOptions(o,lastOptions_);
 if(changed){++generation_;pending_.reset();state_.latest=HudReading{};lastSequence_=0;lastOptions_=o;haveOptions_=true;}
 if(!force&&!changed&&(f->sequence==lastSequence_||std::chrono::duration_cast<std::chrono::milliseconds>(now-submitted_).count()<o.intervalMs))return;
 if(!worker_.joinable()){stop_=false;worker_=std::thread([this]{run();});}
 if(pending_)++state_.replaced;pending_=Job{std::move(f),o,source,generation_};lastSequence_=pending_->frame->sequence;submitted_=now;wake_.notify_one();
}
void HudOcrService::run()noexcept{
 std::unique_ptr<HudTextReader> reader;
 while(true){Job job;{std::unique_lock<std::mutex> lock(mutex_);wake_.wait(lock,[&]{return stop_||pending_.has_value();});if(stop_)break;job=std::move(*pending_);pending_.reset();state_.busy=true;state_.status="reading";}
  try{if(!reader)reader=makeHudTextReader();auto begin=std::chrono::steady_clock::now();auto pixels=cropHudFrame(*job.frame,job.options);auto text=reader->recognize(pixels);auto result=interpretHudText(text,job.options.mode);result.region=job.options.region;result.source=job.source;result.sequence=job.frame->sequence;result.timestamp100ns=job.frame->sourceTimestamp100ns;result.latencyMs=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count();std::lock_guard<std::mutex> lock(mutex_);if(job.generation==generation_&&!stop_){state_.available=true;state_.backend=reader->description();state_.latest=result;state_.status=result.status;state_.message="Actual OCR text; parsed values are unverified candidates";++state_.completed;history_.push_back(std::move(result));if(history_.size()>600)history_.pop_front();}}
  catch(const std::exception& e){Log::write("WARN",std::string("HUD OCR: ")+e.what());std::lock_guard<std::mutex> lock(mutex_);if(job.generation==generation_){state_.available=false;state_.status="unavailable";state_.message=e.what();}reader.reset();}
  {std::lock_guard<std::mutex> lock(mutex_);state_.busy=false;}
 }
}
}
