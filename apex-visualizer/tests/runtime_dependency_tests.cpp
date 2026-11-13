#include "apex/runtime_dependencies.hpp"
#include <onnxruntime/onnxruntime_c_api.h>
#include <iostream>
#include <filesystem>
#include <stdexcept>
struct OrtProbe {
 HMODULE dll=nullptr;const OrtApi* api=nullptr;OrtEnv* env=nullptr;OrtCUDAProviderOptionsV2* options=nullptr;OrtAllocator* allocator=nullptr;char* text=nullptr;
 ~OrtProbe(){if(text&&allocator)allocator->Free(allocator,text);if(options&&api)api->ReleaseCUDAProviderOptions(options);if(env&&api)api->ReleaseEnv(env);if(dll)FreeLibrary(dll);}
 void check(OrtStatus* status){if(status){std::string error=api->GetErrorMessage(status);api->ReleaseStatus(status);throw std::runtime_error(error);}}
};
int wmain(int argc,wchar_t** argv){try{
 if(argc!=2)throw std::runtime_error("Supply a runtime directory for load-only validation");
 apex::RuntimeDependencies dependencies(argv[1]);auto entries=dependencies.inspect();if(entries.size()!=5)throw std::runtime_error("Wrong dependency inventory size");
 for(const auto& d:entries)if(d.state!="found_x64")throw std::runtime_error(d.name+": "+d.state+" / "+d.message);
 dependencies.load();for(const auto& d:dependencies.diagnostics())if(d.state!="loaded")throw std::runtime_error("Dependency not actually loaded");std::cout<<"PASS: five pinned x64 NVIDIA dependency DLLs actually loaded\n";
 OrtProbe ort;ort.dll=LoadLibraryExW((std::filesystem::path(argv[1])/L"onnxruntime.dll").c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);if(!ort.dll)throw std::runtime_error(apex::windowsLoadError(GetLastError()));
 using Base=const OrtApiBase*(ORT_API_CALL*)();auto base=reinterpret_cast<Base>(GetProcAddress(ort.dll,"OrtGetApiBase"));if(!base)throw std::runtime_error("Missing OrtGetApiBase");ort.api=base()->GetApi(ORT_API_VERSION);if(!ort.api)throw std::runtime_error("C API v22 missing");
 ort.check(ort.api->CreateEnv(ORT_LOGGING_LEVEL_WARNING,"dependency-probe",&ort.env));ort.check(ort.api->CreateCUDAProviderOptions(&ort.options));ort.check(ort.api->GetAllocatorWithDefaultOptions(&ort.allocator));
 // This goes through ORT's Provider_SetHost / provider initialization sequence.
 // Querying default options loads the EP without choosing a device, allocating
 // GPU buffers, creating an inference session or claiming successful computation.
 auto status=ort.api->GetCUDAProviderOptionsAsString(ort.options,ort.allocator,&ort.text);
 if(status){std::string error=ort.api->GetErrorMessage(status);ort.api->ReleaseStatus(status);HMODULE driver=LoadLibraryExW(L"nvcuda.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);if(!driver&&error.find("1114")!=std::string::npos){std::cout<<"BLOCKED: ONNX CUDA provider initialization error 1114 and System32 NVIDIA driver is not loadable; provider/inference NOT verified. "<<error<<'\n';return 77;}if(driver)FreeLibrary(driver);throw std::runtime_error(error);}
 auto provider=GetModuleHandleW(L"onnxruntime_providers_cuda.dll");if(!provider)throw std::runtime_error("ORT did not actually load CUDA provider");
 std::cout<<"PASS: pinned x64 dependency DLLs and ONNX CUDA provider loaded through ORT bridge; GPU execution/inference NOT tested\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
