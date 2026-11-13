#include "apex/runtime_dependencies.hpp"
#include <iostream>
#include <filesystem>
#include <stdexcept>
int wmain(int argc,wchar_t** argv){try{
 if(argc!=2)throw std::runtime_error("Supply a runtime directory for load-only validation");
 apex::RuntimeDependencies dependencies(argv[1]);auto entries=dependencies.inspect();if(entries.size()!=5)throw std::runtime_error("Wrong dependency inventory size");
 for(const auto& d:entries)if(d.state!="found_x64")throw std::runtime_error(d.name+": "+d.state+" / "+d.message);
 dependencies.load();for(const auto& d:dependencies.diagnostics())if(d.state!="loaded")throw std::runtime_error("Dependency not actually loaded");
 HMODULE shared=LoadLibraryExW((std::filesystem::path(argv[1])/L"onnxruntime_providers_shared.dll").c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);if(!shared)throw std::runtime_error(apex::windowsLoadError(GetLastError()));
 HMODULE provider=LoadLibraryExW((std::filesystem::path(argv[1])/L"onnxruntime_providers_cuda.dll").c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);unsigned error=GetLastError();if(provider)FreeLibrary(provider);FreeLibrary(shared);if(!provider)throw std::runtime_error(apex::windowsLoadError(error));
 std::cout<<"PASS: actual Windows x64 dependency and CUDA EP DLL loading; no GPU execution or inference claimed\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
