#include "apex/compute.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <limits>
#include <stdexcept>
#define CHECK(x) do{if(!(x))throw std::runtime_error(#x);}while(false)
int main(){auto dir=std::filesystem::temp_directory_path()/"apex-compute-unit-test";try{
 auto values=apex::computeReference();std::string error;CHECK(values.size()==32768);CHECK(apex::verifyComputeOutput(values.data(),values.size(),error));values[20]+=1;CHECK(!apex::verifyComputeOutput(values.data(),values.size(),error));values[20]=std::numeric_limits<float>::quiet_NaN();CHECK(!apex::verifyComputeOutput(values.data(),values.size(),error));CHECK(!apex::verifyComputeOutput(nullptr,0,error));
 apex::TimingSummary t;t.add(1);t.add(3);CHECK(t.count==2&&t.mean()==2&&t.min==1&&t.max==3);bool threw=false;try{t.add(-1);}catch(...){threw=true;}CHECK(threw);
 apex::ComputeSnapshot s;s.finished=true;s.phase="unavailable";s.cuda.state="unavailable";s.gpu.state="unavailable";apex::saveComputeReport(dir,s);nlohmann::json report;std::ifstream(dir/"compute-report.json")>>report;CHECK(report["cuda_workload"]["verified"]==false);CHECK(report["cuda_ep_inference"]["run_end_to_end"].is_null());CHECK(report["cpu_to_cuda_ep_latency_ratio"].is_null());CHECK(report["test_inventory"].size()==6);CHECK(report["runtime_path"]=="");CHECK(report["available_providers"].empty());CHECK(report["process_before"]["process_busiest_engine_percent"].is_null());
 s.cpu.verified=true;s.cpu.state="passed";s.cpu.latency.add(1);s.phase="completed";CHECK(apex::computeRunSucceeded(s,false));CHECK(!apex::computeRunSucceeded(s,true));s.phase="cancelled";CHECK(!apex::computeRunSucceeded(s,false));s.phase="completed";s.gpu.state="failed";CHECK(!apex::computeRunSucceeded(s,false));
 std::filesystem::remove_all(dir);std::cout<<"PASS: independent ONNX numerical reference, mismatch/NaN/shape rejection, timing aggregation, unavailable status/report without fabricated counters\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';std::error_code ec;std::filesystem::remove_all(dir,ec);return 1;}}
