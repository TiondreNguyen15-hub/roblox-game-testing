#pragma once
#include "telemetry.hpp"
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
namespace apex {
// Only the worker touches disk/parser state; front_* are owned by the UI thread.
class AsyncFileSource final:public IDataSource {
public:
 AsyncFileSource(){worker_=std::thread([this]{run();});}
 ~AsyncFileSource()override{{std::lock_guard<std::mutex> lock(mutex_);stop_=true;}wake_.notify_one();if(worker_.joinable())worker_.join();}
 AsyncFileSource(const AsyncFileSource&)=delete;AsyncFileSource& operator=(const AsyncFileSource&)=delete;
 double processingMs=-1,deliveryMs=-1;
 void reset(){processingMs=deliveryMs=-1;std::lock_guard<std::mutex> lock(mutex_);++generation_;front_.clear();frontStatus_="WAITING FOR JSON DATA";lastSent_=-1;outReady_=false;}
 void poll(const std::filesystem::path& path,double now)override{
  std::lock_guard<std::mutex> lock(mutex_);
  if(path!=frontPath_){processingMs=deliveryMs=-1;frontPath_=path;++generation_;front_.clear();frontStatus_="WAITING FOR JSON DATA";lastSent_=-1;outReady_=false;}
  if(outReady_&&outGeneration_==generation_){front_=std::move(out_);frontStatus_=std::move(outStatus_);processingMs=outProcessing_;deliveryMs=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-outStart_).count();outReady_=false;}
  if(lastSent_<0||now-lastSent_>=.2){requestPath_=path;requestTime_=now;requestGeneration_=generation_;pending_=true;lastSent_=now;wake_.notify_one();}
 }
 const std::vector<Entity>& entities()const override{return front_;}
 std::string status(double)const override{return frontStatus_;}
private:
 void run()noexcept{
  Telemetry source;unsigned long long current=0;
  for(;;){
   std::filesystem::path path;double now;unsigned long long generation;
   {std::unique_lock<std::mutex> lock(mutex_);wake_.wait(lock,[this]{return stop_||pending_;});if(stop_)return;path=requestPath_;now=requestTime_;generation=requestGeneration_;pending_=false;}
   try{
    if(current!=generation){source.reset();current=generation;}
    auto start=std::chrono::steady_clock::now();source.poll(path,now);auto end=std::chrono::steady_clock::now();auto frame=source.entities();auto message=source.status(now);
    std::lock_guard<std::mutex> lock(mutex_);if(generation==generation_){outStart_=start;outProcessing_=std::chrono::duration<double,std::milli>(end-start).count();out_=std::move(frame);outStatus_=std::move(message);outGeneration_=generation;outReady_=true;}
   }catch(...){try{std::lock_guard<std::mutex> lock(mutex_);if(generation==generation_){outProcessing_=-1;outStart_=std::chrono::steady_clock::now();outStatus_="FILE ERROR / reader failed";out_=source.entities();outGeneration_=generation;outReady_=true;}}catch(...){}}
  }
 }
 std::mutex mutex_;std::condition_variable wake_;bool stop_=false,pending_=false,outReady_=false;
 unsigned long long generation_=1,requestGeneration_=0,outGeneration_=0;double requestTime_=0,lastSent_=-1;
 std::filesystem::path frontPath_,requestPath_;std::vector<Entity> front_,out_;std::string frontStatus_="WAITING FOR JSON DATA",outStatus_;
 double outProcessing_=-1;std::chrono::steady_clock::time_point outStart_{};
 std::thread worker_;
};
}
