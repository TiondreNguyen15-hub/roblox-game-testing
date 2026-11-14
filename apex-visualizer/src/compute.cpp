#include "apex/compute.hpp"
#include "apex/log.hpp"
#include "apex/runtime_dependencies.hpp"
#include <cstring>
#include <windows.h>
#include <bcrypt.h>
#include <onnxruntime/onnxruntime_c_api.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <cctype>
#include <functional>
namespace apex {
namespace {
using Clock=std::chrono::steady_clock;
double elapsed(Clock::time_point start){return std::chrono::duration<double>(Clock::now()-start).count();}
std::string lower(std::string v){for(char& c:v)c=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));return v;}
struct Unavailable:std::runtime_error{std::string reason;explicit Unavailable(const std::string& message,std::string code="UNAVAILABLE"):std::runtime_error(message),reason(std::move(code)){};};
struct Skipped:Unavailable{using Unavailable::Unavailable;};
struct Cancelled:std::runtime_error{Cancelled():std::runtime_error("Cancelled; partial measurements only"){};};
void cancelled(const std::atomic<bool>& stop){if(stop)throw Cancelled();}
template<class T>T proc(HMODULE dll,const char* name){auto fn=reinterpret_cast<T>(GetProcAddress(dll,name));if(!fn)throw Unavailable(std::string("Missing API: ")+name);return fn;}
const char* vectorPtx=R"ptx(
.version 6.0
.target sm_52
.address_size 64
.visible .entry vector_add(.param .u64 a,.param .u64 b,.param .u64 c,.param .u32 n){
 .reg .pred %p; .reg .b32 %r<6>; .reg .b64 %rd<8>; .reg .f32 %f<4>;
 ld.param.u64 %rd1,[a];ld.param.u64 %rd2,[b];ld.param.u64 %rd3,[c];ld.param.u32 %r1,[n];
 mov.u32 %r2,%ctaid.x;mov.u32 %r3,%ntid.x;mov.u32 %r4,%tid.x;
 mad.lo.s32 %r5,%r2,%r3,%r4;setp.ge.u32 %p,%r5,%r1;@%p bra DONE;
 mul.wide.u32 %rd4,%r5,4;add.s64 %rd5,%rd1,%rd4;add.s64 %rd6,%rd2,%rd4;add.s64 %rd7,%rd3,%rd4;
 ld.global.f32 %f1,[%rd5];ld.global.f32 %f2,[%rd6];add.f32 %f3,%f1,%f2;st.global.f32 [%rd7],%f3;
DONE: ret;
}
)ptx";
struct Cuda {
 HMODULE dll=nullptr;void* context=nullptr;void* module=nullptr;std::array<unsigned long long,3> memory{};void* begin=nullptr;void* end=nullptr;int ordinal=-1,device=-1,driverVersion=0;
 using Destroy=int(__stdcall*)(void*);using Free=int(__stdcall*)(unsigned long long);
 Destroy destroyContext=nullptr,unload=nullptr,destroyEvent=nullptr;Free freeMemory=nullptr;
 ~Cuda(){if(destroyEvent){if(end)destroyEvent(end);if(begin)destroyEvent(begin);}if(freeMemory)for(auto p:memory)if(p)freeMemory(p);if(module&&unload)unload(module);if(context&&destroyContext)destroyContext(context);if(dll)FreeLibrary(dll);}
 void check(int code,const char* stage){if(code){using Error=int(__stdcall*)(int,const char**);const char* text=nullptr;auto f=reinterpret_cast<Error>(GetProcAddress(dll,"cuGetErrorString"));if(f)f(code,&text);throw std::runtime_error(std::string(stage)+" failed: "+std::to_string(code)+(text?" / "+std::string(text):""));}}
 void initialize(const ComputeOptions& options){
  dll=LoadLibraryExW(L"nvcuda.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);if(!dll)throw Unavailable("CUDA driver DLL unavailable: "+windowsLoadError(GetLastError()),"CUDA_DRIVER_NOT_LOADABLE");
  using Init=int(__stdcall*)(unsigned);using Count=int(__stdcall*)(int*);using Name=int(__stdcall*)(char*,int,int);using Device=int(__stdcall*)(int*,int);using Version=int(__stdcall*)(int*);using Luid=int(__stdcall*)(char*,unsigned*,int);
  auto init=proc<Init>(dll,"cuInit");int code=init(0);if(code)throw Unavailable("cuInit failed: "+std::to_string(code),"CUDA_DRIVER_INIT_FAILED");
  check(proc<Version>(dll,"cuDriverGetVersion")(&driverVersion),"CUDA driver version");int count=0;check(proc<Count>(dll,"cuDeviceGetCount")(&count),"CUDA enumerate");
  auto getLuid=reinterpret_cast<Luid>(GetProcAddress(dll,"cuDeviceGetLuid"));std::vector<std::pair<int,int>> names,luids;bool hasLuids=false;
  for(int i=0;i<count;++i){int candidate=-1;check(proc<Device>(dll,"cuDeviceGet")(&candidate,i),"CUDA device ordinal");char name[256]{};
   if(proc<Name>(dll,"cuDeviceGetName")(name,256,candidate)==0&&lower(name)==lower(options.adapter))names.push_back({i,candidate});
   char bytes[8]{};unsigned mask=0;if(getLuid&&getLuid(bytes,&mask,candidate)==0){hasLuids=true;std::uint64_t id=0;std::memcpy(&id,bytes,8);if(id==options.adapterLuid)luids.push_back({i,candidate});}
  }
  const auto& matches=options.adapterLuid&&hasLuids?luids:names;
  if(matches.size()!=1)throw Unavailable("Selected DXGI adapter does not uniquely match a CUDA device by LUID (or exact name when LUID is unsupported)","CUDA_DEVICE_NOT_MATCHED");ordinal=matches[0].first;device=matches[0].second;
  destroyContext=proc<Destroy>(dll,"cuCtxDestroy_v2");unload=proc<Destroy>(dll,"cuModuleUnload");destroyEvent=proc<Destroy>(dll,"cuEventDestroy_v2");freeMemory=proc<Free>(dll,"cuMemFree_v2");
  using Create=int(__stdcall*)(void**,unsigned,int);check(proc<Create>(dll,"cuCtxCreate_v2")(&context,0,device),"CUDA context creation");
 }
};
ComputeTest cudaWork(const ComputeOptions& o,std::atomic<bool>& stop,int& device,int& driverVersion){
 ComputeTest result;Cuda cuda;cuda.initialize(o);device=cuda.ordinal;driverVersion=cuda.driverVersion;
 using Load=int(__stdcall*)(void**,const void*);cuda.check(proc<Load>(cuda.dll,"cuModuleLoadData")(&cuda.module,vectorPtx),"PTX JIT/module load");
 using Function=int(__stdcall*)(void**,void*,const char*);void* function=nullptr;cuda.check(proc<Function>(cuda.dll,"cuModuleGetFunction")(&function,cuda.module,"vector_add"),"CUDA function");
 using Alloc=int(__stdcall*)(unsigned long long*,size_t);using HtoD=int(__stdcall*)(unsigned long long,const void*,size_t);using DtoH=int(__stdcall*)(void*,unsigned long long,size_t);
 using EventCreate=int(__stdcall*)(void**,unsigned);using Record=int(__stdcall*)(void*,void*);using Sync=int(__stdcall*)(void*);using Time=int(__stdcall*)(float*,void*,void*);
 using Launch=int(__stdcall*)(void*,unsigned,unsigned,unsigned,unsigned,unsigned,unsigned,unsigned,void*,void**,void**);
 auto alloc=proc<Alloc>(cuda.dll,"cuMemAlloc_v2");auto htod=proc<HtoD>(cuda.dll,"cuMemcpyHtoD_v2");auto dtoh=proc<DtoH>(cuda.dll,"cuMemcpyDtoH_v2");auto record=proc<Record>(cuda.dll,"cuEventRecord");auto sync=proc<Sync>(cuda.dll,"cuEventSynchronize");auto time=proc<Time>(cuda.dll,"cuEventElapsedTime");auto launch=proc<Launch>(cuda.dll,"cuLaunchKernel");
 const unsigned n=1u<<20;const size_t bytes=size_t(n)*sizeof(float);std::vector<float> a(n),b(n),out(n);for(unsigned i=0;i<n;++i){a[i]=float(i%1024)*.125f;b[i]=-float(i%31)*.25f;}
 for(auto& p:cuda.memory)cuda.check(alloc(&p,bytes),"CUDA allocation");auto create=proc<EventCreate>(cuda.dll,"cuEventCreate");cuda.check(create(&cuda.begin,0),"CUDA event");cuda.check(create(&cuda.end,0),"CUDA event");
 auto loop=[&](bool measure){cancelled(stop);auto start=Clock::now();cuda.check(htod(cuda.memory[0],a.data(),bytes),"HtoD A");cuda.check(htod(cuda.memory[1],b.data(),bytes),"HtoD B");cuda.check(record(cuda.begin,nullptr),"CUDA start event");unsigned size=n;void* args[]={&cuda.memory[0],&cuda.memory[1],&cuda.memory[2],&size};cuda.check(launch(function,(n+255)/256,1,1,256,1,1,0,nullptr,args,nullptr),"CUDA kernel launch");cuda.check(record(cuda.end,nullptr),"CUDA end event");cuda.check(sync(cuda.end),"CUDA completion");float ms=0;cuda.check(time(&ms,cuda.begin,cuda.end),"CUDA elapsed time");cuda.check(dtoh(out.data(),cuda.memory[2],bytes),"DtoH result");double wall=elapsed(start)*1000.;
  for(unsigned i=0;i<n;++i)if(!std::isfinite(out[i])||out[i]!=a[i]+b[i])throw std::runtime_error("CUDA vector-add numerical mismatch at "+std::to_string(i));
  if(measure){result.latency.add(wall);result.kernel.add(ms);}
 };
 for(int i=0;i<10;++i)loop(false);auto start=Clock::now();do{loop(true);}while(result.latency.count<static_cast<unsigned>(o.iterations)||elapsed(start)<o.phaseSeconds);
 result.measuredWallSeconds=elapsed(start);result.reason="VERIFIED_EXECUTION";result.verified=true;result.state="passed";result.message="Actual PTX JIT, HtoD, kernel launch, GPU events, completion and DtoH verified against all 1,048,576 reference values; 12 MiB device buffers released after phase";return result;
}
struct OrtLibrary {
 HMODULE dll=nullptr;const OrtApi* api=nullptr;OrtEnv* env=nullptr;std::string version,path;int driverVersion=0;std::vector<std::string> providers;std::unique_ptr<RuntimeDependencies> dependencies;std::filesystem::path folder;
 ~OrtLibrary(){if(env&&api)api->ReleaseEnv(env);if(dll)FreeLibrary(dll);}
 void check(OrtStatus* status){if(status){std::string message=api->GetErrorMessage(status);api->ReleaseStatus(status);throw std::runtime_error("ONNX Runtime: "+message);}}
 static void ORT_API_CALL logger(void*,OrtLoggingLevel,const char*,const char*,const char*,const char* message){try{Log::write("WARN",std::string("ONNX Runtime: ")+(message?message:""));}catch(...){}}
 void initialize(const std::filesystem::path& base){
  std::string errors;
  for(const auto& directory:{base/L"runtime/gpu",base/L"runtime/cpu"}){auto file=directory/L"onnxruntime.dll";if(!std::filesystem::exists(file))continue;dll=LoadLibraryExW(file.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);if(dll){folder=directory;break;}errors+=" "+directory.u8string()+" Win32="+std::to_string(GetLastError());}
  if(!dll)throw Unavailable("ONNX Runtime not loadable. Install bundled CPU runtime VC++ x64 dependencies or GPU runtime/CUDA dependencies."+errors,"ORT_RUNTIME_NOT_LOADABLE");
  using GetBase=const OrtApiBase*(ORT_API_CALL*)();const auto* b=proc<GetBase>(dll,"OrtGetApiBase")();api=b->GetApi(ORT_API_VERSION);if(!api)throw Unavailable("ONNX Runtime C API v22 unavailable (use runtime 1.22.x)");version=b->GetVersionString();path=loadedModulePath(dll);folder=std::filesystem::u8path(path).parent_path();check(api->CreateEnvWithCustomLogger(logger,nullptr,ORT_LOGGING_LEVEL_WARNING,"ApexComputeValidation",&env));
  char** names=nullptr;int count=0;check(api->GetAvailableProviders(&names,&count));for(int i=0;i<count;++i)providers.emplace_back(names[i]);check(api->ReleaseAvailableProviders(names,count));dependencies=std::make_unique<RuntimeDependencies>(folder);dependencies->inspect();
 }
 void preloadCuda(){
  try{dependencies->load();}catch(const std::exception& e){throw Unavailable(e.what(),"ORT_CUDA_DEPENDENCY_LOAD_FAILED");}
  // Provider DLLs must be initialized by ORT's bridge. Loading CUDA directly
  // before Provider_SetHost(shared, host) can fail its DLL initialization (1114).
  // We preload NVIDIA dependencies only, then use the public ORT C API below.

 }
 bool hasCuda()const{return std::find(providers.begin(),providers.end(),"CUDAExecutionProvider")!=providers.end();}

};
struct OrtSessionOwner {
 OrtLibrary& lib;OrtSessionOptions* options=nullptr;OrtCUDAProviderOptionsV2* cudaOptions=nullptr;OrtSession* session=nullptr;OrtMemoryInfo* memory=nullptr;OrtValue* input=nullptr;OrtValue* output=nullptr;
 explicit OrtSessionOwner(OrtLibrary& l):lib(l){}
 ~OrtSessionOwner(){auto a=lib.api;if(output)a->ReleaseValue(output);if(input)a->ReleaseValue(input);if(memory)a->ReleaseMemoryInfo(memory);if(session)a->ReleaseSession(session);if(cudaOptions)a->ReleaseCUDAProviderOptions(cudaOptions);if(options)a->ReleaseSessionOptions(options);}
};
ComputeTest inference(OrtLibrary& lib,const ComputeOptions& o,bool gpu,int device,std::atomic<bool>& stop,const std::function<void(bool)>& active){
 auto a=lib.api;ComputeTest result;OrtSessionOwner s(lib);lib.check(a->CreateSessionOptions(&s.options));lib.check(a->SetIntraOpNumThreads(s.options,1));lib.check(a->SetInterOpNumThreads(s.options,1));lib.check(a->SetSessionGraphOptimizationLevel(s.options,ORT_ENABLE_ALL));
 if(gpu){if(device<0)throw Skipped("No verified CUDA device/context; ONNX GPU inference was not attempted. See CUDA workload result.","BLOCKED_BY_CUDA_DEVICE");if(!lib.hasCuda())throw Unavailable("Loaded CPU-only ONNX Runtime at "+lib.path+"; no CUDAExecutionProvider. Run setup-gpu-runtime.ps1 -InstallDependencies, restart, then validate.","ORT_CPU_ONLY_RUNTIME");if(lib.driverVersion>0&&lib.driverVersion<12000)throw Unavailable("ONNX Runtime 1.22 CUDA EP requires a CUDA 12 compatible NVIDIA driver; detected driver API "+std::to_string(lib.driverVersion),"CUDA_DRIVER_TOO_OLD");lib.preloadCuda();lib.check(a->CreateCUDAProviderOptions(&s.cudaOptions));std::string ordinal=std::to_string(device);const char* keys[]={"device_id","use_tf32","do_copy_in_default_stream","gpu_mem_limit","arena_extend_strategy"};const char* values[]={ordinal.c_str(),"0","1","536870912","kSameAsRequested"};lib.check(a->UpdateCUDAProviderOptions(s.cudaOptions,keys,values,5));lib.check(a->SessionOptionsAppendExecutionProvider_CUDA_V2(s.options,s.cudaOptions));lib.check(a->AddSessionConfigEntry(s.options,"session.disable_cpu_ep_fallback","1"));auto prefix=o.resultDirectory/L"cuda-profile";lib.check(a->EnableProfiling(s.options,prefix.c_str()));}
 auto model=o.baseDirectory/L"models/compute_validation.onnx";lib.check(a->CreateSession(lib.env,model.c_str(),s.options,&s.session));auto input=computeInput();const int64_t shape[]={128,256};lib.check(a->CreateCpuMemoryInfo(OrtArenaAllocator,OrtMemTypeDefault,&s.memory));lib.check(a->CreateTensorWithDataAsOrtValue(s.memory,input.data(),input.size()*sizeof(float),shape,2,ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT,&s.input));const char* inputs[]={"input"};const char* outputs[]={"output"};
 auto loop=[&](bool measure){cancelled(stop);if(s.output){a->ReleaseValue(s.output);s.output=nullptr;}auto start=Clock::now();const OrtValue* in[]={s.input};lib.check(a->Run(s.session,nullptr,inputs,in,1,outputs,1,&s.output));const OrtMemoryInfo* outputMemory=nullptr;lib.check(a->GetTensorMemoryInfo(s.output,&outputMemory));OrtMemoryInfoDeviceType deviceType; a->MemoryInfoGetDeviceType(outputMemory,&deviceType);if(deviceType!=OrtMemoryInfoDeviceType_CPU)throw std::runtime_error("ONNX output is not in CPU memory; cannot validate safely");void* data=nullptr;lib.check(a->GetTensorMutableData(s.output,&data));double wall=elapsed(start)*1000.;OrtTensorTypeAndShapeInfo* info=nullptr;lib.check(a->GetTensorTypeAndShape(s.output,&info));size_t count=0;ONNXTensorElementDataType elementType;auto status=a->GetTensorShapeElementCount(info,&count);auto typeStatus=a->GetTensorElementType(info,&elementType);a->ReleaseTensorTypeAndShapeInfo(info);lib.check(status);lib.check(typeStatus);if(elementType!=ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)throw std::runtime_error("Expected float32 ONNX output");std::string error;if(!verifyComputeOutput(static_cast<const float*>(data),count,error))throw std::runtime_error(error);if(measure)result.latency.add(wall);};
 for(int i=0;i<10;++i)loop(false);
 if(gpu){OrtAllocator* allocator=nullptr;lib.check(a->GetAllocatorWithDefaultOptions(&allocator));char* path=nullptr;lib.check(a->SessionEndProfiling(s.session,allocator,&path));std::string profile=path?path:"";result.profileFile=profile;if(path)allocator->Free(allocator,path);std::ifstream file(std::filesystem::u8path(profile));nlohmann::json events;file>>events;bool executed=false;if(events.is_array())for(const auto& event:events){if(event.value("cat",std::string{})=="Node"&&event.contains("args")&&event["args"].is_object()&&event["args"].value("provider",std::string{})=="CUDAExecutionProvider"){auto op=event["args"].value("op_name",std::string{});if(op.find("MatMul")!=std::string::npos||op.find("Gemm")!=std::string::npos||op=="Relu")executed=true;}}if(!executed)throw std::runtime_error("No actual CUDAExecutionProvider node found in ORT profiling; acceleration not verified");}
 active(gpu);auto start=Clock::now();do{loop(true);}while(result.latency.count<static_cast<unsigned>(o.iterations)||elapsed(start)<o.phaseSeconds);
 result.measuredWallSeconds=elapsed(start);result.reason="VERIFIED_EXECUTION";result.verified=true;result.state="passed";result.message=gpu?"CUDA EP execution proved by actual node profile and numerical reference; CPU EP fallback disabled; TF32 off":"CPUExecutionProvider Run results agree with independent numerical reference; one intra/inter-op thread";return result;
}
#include "model_benchmark.inc"
}
ComputeRunner::~ComputeRunner(){stop();}
void ComputeRunner::stop(){cancel_=true;if(worker_.joinable())worker_.join();}
ComputeSnapshot ComputeRunner::snapshot()const{std::lock_guard<std::mutex> lock(mutex_);return state_;}
void ComputeRunner::phase(const std::string& name){std::lock_guard<std::mutex> lock(mutex_);state_.phase=name;phaseStart_=Clock::now();if(name=="cuda_vector_add")state_.cuda.state="running";if(name=="cpu_inference")state_.cpu.state="running";if(name=="cuda_ep_inference")state_.gpu.state="running";}
void ComputeRunner::progress(const ComputeTest& test,bool gpu){std::lock_guard<std::mutex> lock(mutex_);(gpu?state_.gpu:state_.cpu)=test;(gpu?state_.gpu:state_.cpu).state="running";}
void ComputeRunner::publish(const ComputeTest& test,int which){std::lock_guard<std::mutex> lock(mutex_);(which==0?state_.cuda:which==1?state_.cpu:state_.gpu)=test;}
void ComputeRunner::start(const ComputeOptions& options){
 if(snapshot().running)return;if(options.iterations<1||options.iterations>100000||!std::isfinite(options.phaseSeconds)||options.phaseSeconds<0||options.phaseSeconds>86400||!std::isfinite(options.cpuSeconds)||options.cpuSeconds<1||options.cpuSeconds>86400||!std::isfinite(options.gpuSeconds)||options.gpuSeconds<1||options.gpuSeconds>86400)throw std::runtime_error("Invalid compute test limits");
 stop();finishCapture();trace_.clear();std::error_code captureError;std::filesystem::create_directories(options.resultDirectory,captureError);if(!captureError)trace_.open(options.resultDirectory/L"compute-samples.csv");if(trace_)trace_<<"elapsed_s,phase,render_frame_interval_ms,process_busiest_engine_percent,process_local_vram_mib,working_set_mib,private_mib,device_gpu_percent,device_vram_mib\n";else Log::write("WARN","Compute process trace unavailable");captureStart_=Clock::now();lastCapture_=-1;
 cancel_=false;{std::lock_guard<std::mutex> lock(mutex_);state_=ComputeSnapshot{};state_.running=true;state_.adapter=options.adapter;state_.before=latest_;state_.peak=latest_;state_.requestedIterations=options.iterations;state_.requestedPhaseSeconds=options.phaseSeconds;state_.modelBenchmark=options.modelBenchmark;state_.gpuTargetSeconds=options.modelBenchmark?options.gpuSeconds:options.phaseSeconds;memorySamples_.clear();}
 try{worker_=std::thread([this,options]{run(options);});}catch(...){std::lock_guard<std::mutex> lock(mutex_);state_.running=false;state_.finished=true;state_.error="Compute worker creation failed";throw;}
}
void ComputeRunner::finishCapture(){if(trace_.is_open()){trace_.flush();if(!trace_)Log::write("WARN","Compute process trace write failed");trace_.close();}}
void ComputeRunner::observe(double frameMs,const ProcessObservation& p){
 std::lock_guard<std::mutex> lock(mutex_);latest_=p;if(!state_.running&&!state_.finished)return;state_.after=p;++state_.observations;
 double age=elapsed(captureStart_);if(trace_.is_open()&&(lastCapture_<0||age-lastCapture_>=.5)){lastCapture_=age;if(state_.phase=="cuda_ep_inference"){double phaseAge=std::chrono::duration<double>(Clock::now()-phaseStart_).count();if(phaseAge>=30){if(memorySamples_.size()>=2400)memorySamples_.erase(memorySamples_.begin());memorySamples_.push_back({phaseAge,p.localVramMiB,p.privateMiB});}state_.vramTrend=assessMemoryTrend(memorySamples_,true);state_.privateTrend=assessMemoryTrend(memorySamples_,false);}
 trace_<<age<<','<<state_.phase<<','<<frameMs;for(double value:{p.engineBusy,p.localVramMiB,p.workingMiB,p.privateMiB,p.deviceBusy,p.deviceVramMiB}){trace_<<',';if(std::isfinite(value)&&value>=0)trace_<<value;}trace_<<'\n';}
 state_.peak.deviceBusy=std::max(state_.peak.deviceBusy,p.deviceBusy);state_.peak.deviceVramMiB=std::max(state_.peak.deviceVramMiB,p.deviceVramMiB);state_.peak.engineBusy=std::max(state_.peak.engineBusy,p.engineBusy);state_.peak.localVramMiB=std::max(state_.peak.localVramMiB,p.localVramMiB);state_.peak.workingMiB=std::max(state_.peak.workingMiB,p.workingMiB);state_.peak.privateMiB=std::max(state_.peak.privateMiB,p.privateMiB);
 if(state_.running&&std::isfinite(frameMs)&&frameMs>0)state_.renderIntervals[state_.phase].add(frameMs);
}
void ComputeRunner::run(ComputeOptions options)noexcept{
 try{
  std::filesystem::create_directories(options.resultDirectory);
  phase("preflight");options.modelPath=normalizeComputePath(options.modelPath,options.baseDirectory);options.inputProfile=normalizeComputePath(options.inputProfile,options.baseDirectory);
  {std::lock_guard<std::mutex> lock(mutex_);state_.modelPath=(options.modelPath.empty()?options.baseDirectory/L"models/compute_validation.onnx":options.modelPath).u8string();state_.inputProfilePath=options.inputProfile.u8string();}
  if(prepareComputeInputs(options))Log::write("INFO","Restored missing bundled validation model: "+options.modelPath.u8string());auto digest=modelDigest(options.modelPath);auto profileDigest=options.inputProfile.empty()?std::string{}:modelDigest(options.inputProfile);{std::lock_guard<std::mutex> lock(mutex_);state_.modelPath=options.modelPath.u8string();state_.modelSha256=digest;state_.inputProfilePath=options.inputProfile.u8string();state_.inputProfileSha256=profileDigest;if(options.modelBenchmark)state_.modelHashDescription="Actual selected ONNX file, deterministic FP32 inputs, CPU-reference comparison";}
  int device=-1,driverVersion=0;
  auto attempt=[&](int which,auto&& action){ComputeTest result;try{result=action();}catch(const Cancelled&){result.state="cancelled";result.reason="CANCELLED";result.message="Cancelled; phase not verified complete";publish(result,which);throw;}catch(const Skipped& e){result.state="skipped";result.reason=e.reason;result.blockedBy=e.reason=="BLOCKED_BY_CPU_REFERENCE"?"cpu_inference":"cuda_workload";result.message=e.what();}catch(const Unavailable& e){result.state="unavailable";result.reason=e.reason;result.message=e.what();}catch(const std::exception& e){result.state="failed";result.reason="EXECUTION_OR_VALIDATION_FAILED";result.message=e.what();}publish(result,which);Log::write(result.verified?"INFO":"WARN","Compute "+std::to_string(which)+" "+result.state+": "+result.message);return result;};
  phase("cuda_vector_add");auto cuda=attempt(0,[&]{return cudaWork(options,cancel_,device,driverVersion);});cancelled(cancel_);
  OrtLibrary lib;try{lib.initialize(options.baseDirectory);{std::lock_guard<std::mutex> lock(mutex_);state_.ortVersion=lib.version;state_.runtimePath=lib.path;state_.availableProviders=lib.providers;state_.dependencies=lib.dependencies->diagnostics();state_.cudaDriverVersion=driverVersion;lib.driverVersion=driverVersion;}}
  catch(const std::exception& e){ComputeTest unavailable;unavailable.state="unavailable";unavailable.reason="ORT_RUNTIME_NOT_LOADABLE";unavailable.message=e.what();publish(unavailable,1);publish(unavailable,2);throw Unavailable(e.what());}
  ReferenceOutputs reference;auto update=[this](const ComputeTest& value,bool gpu){progress(value,gpu);};
  auto active=[this](bool gpu){std::lock_guard<std::mutex> lock(mutex_);phaseStart_=Clock::now();if(gpu)memorySamples_.clear();state_.providerStatus=gpu?"CUDA EP ACTIVE (profile + numerical test passed); benchmark running":"CPU ACTIVE (numerical test passed); CUDA EP evaluation pending";};
  phase("cpu_inference");auto cpu=attempt(1,[&]{return options.modelBenchmark?benchmarkInference(lib,options,false,device,cancel_,reference,active,update):inference(lib,options,false,device,cancel_,active);});cancelled(cancel_);
  phase("cuda_ep_inference");auto gpu=attempt(2,[&]{if(options.modelBenchmark&&!cpu.verified)throw Skipped("CPU reference benchmark did not pass","BLOCKED_BY_CPU_REFERENCE");return options.modelBenchmark?benchmarkInference(lib,options,true,device,cancel_,reference,active,update):inference(lib,options,true,device,cancel_,active);});
  if((!options.inputProfile.empty()&&modelDigest(options.inputProfile)!=profileDigest)||modelDigest(options.modelPath)!=digest)throw std::runtime_error("Model file changed during CPU/GPU benchmark");
  {std::lock_guard<std::mutex> lock(mutex_);state_.dependencies=lib.dependencies->diagnostics();state_.providerStatus=gpu.verified?"CUDA EP VERIFIED (test completed; inference stopped)":cpu.verified?"CPU fallback verified; CUDA EP "+gpu.state+" ["+gpu.reason+"]: "+gpu.message:"Inference unavailable: no verified provider";state_.phase="completed";}
 }catch(const Cancelled& e){std::lock_guard<std::mutex> lock(mutex_);state_.phase="cancelled";state_.error=e.what();state_.providerStatus="Cancelled; inspect individual verified results";}
 catch(const ComputeInputError& e){std::lock_guard<std::mutex> lock(mutex_);markComputeStartupFailure(state_,e.code,e.what());Log::write("ERROR","Compute startup ["+e.code+"]: "+e.what());}
 catch(const std::exception& e){std::lock_guard<std::mutex> lock(mutex_);markComputeStartupFailure(state_,"COMPUTE_STARTUP_FAILED",e.what());Log::write("ERROR",std::string("Compute: ")+e.what());}
 catch(...){std::lock_guard<std::mutex> lock(mutex_);state_.phase="failed";state_.error="Unexpected compute failure";state_.providerStatus="Compute failure; DX11 renderer remains active";}
 std::lock_guard<std::mutex> lock(mutex_);state_.running=false;state_.finished=true;
}
}
