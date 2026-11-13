#include "apex/runtime_dependencies.hpp"
#include <algorithm>
#include <cwctype>
#include <fstream>
#include <set>
#include <stdexcept>
namespace apex {
namespace {
std::wstring environment(const wchar_t* name){DWORD length=GetEnvironmentVariableW(name,nullptr,0);if(!length)return {};std::wstring v(length,L'\0');DWORD n=GetEnvironmentVariableW(name,v.data(),length);if(!n||n>=length)return {};v.resize(n);return v;}
std::wstring lower(std::wstring s){for(auto& c:s)c=static_cast<wchar_t>(std::towlower(c));return s;}
void add(std::vector<std::filesystem::path>& dirs,std::filesystem::path p){
 if(p.empty())return;std::error_code ec;if(!p.is_absolute()||!std::filesystem::is_directory(p,ec))return;p=p.lexically_normal();for(const auto& d:dirs)if(lower(d.wstring())==lower(p.wstring()))return;dirs.push_back(p);
}
void split(std::vector<std::filesystem::path>& dirs,const std::wstring& list){size_t start=0;while(start<=list.size()){auto end=list.find(L';',start);auto part=list.substr(start,end-start);if(part.size()>1&&part.front()==L'"'&&part.back()==L'"')part=part.substr(1,part.size()-2);add(dirs,part);if(end==std::wstring::npos)break;start=end+1;}}
bool x64(const std::filesystem::path& file){std::ifstream in(file,std::ios::binary);unsigned short mz=0,machine=0;unsigned offset=0,signature=0;in.read(reinterpret_cast<char*>(&mz),2);in.seekg(0x3c);in.read(reinterpret_cast<char*>(&offset),4);if(!in||mz!=0x5a4d||offset>64*1024*1024)return false;in.seekg(offset);in.read(reinterpret_cast<char*>(&signature),4);in.read(reinterpret_cast<char*>(&machine),2);return bool(in)&&signature==0x4550&&machine==0x8664;}
const wchar_t* required[]={L"cudart64_12.dll",L"cublasLt64_12.dll",L"cublas64_12.dll",L"cufft64_11.dll",L"cudnn64_9.dll"};
}
std::string loadedModulePath(HMODULE module){wchar_t path[32768]{};DWORD n=GetModuleFileNameW(module,path,32768);return n&&n<32768?std::filesystem::path(path).u8string():std::string{};}
std::string windowsLoadError(unsigned long code){char* value=nullptr;DWORD n=FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,nullptr,code,0,reinterpret_cast<char*>(&value),0,nullptr);std::string text=n&&value?std::string(value,n):"Unknown Windows loader error";if(value)LocalFree(value);for(char& c:text)if(c=='\r'||c=='\n')c=' ';return "Win32 "+std::to_string(code)+": "+text;}
RuntimeDependencies::RuntimeDependencies(std::filesystem::path folder){
 add(directories_,std::filesystem::absolute(folder));split(directories_,environment(L"APEX_CUDA_DLL_DIRS"));
 auto cuda=environment(L"CUDA_PATH");if(!cuda.empty())add(directories_,std::filesystem::path(cuda)/L"bin");
 auto cudnn=environment(L"CUDNN_PATH");if(!cudnn.empty()){add(directories_,cudnn);add(directories_,std::filesystem::path(cudnn)/L"bin");}
 // PATH is inspected as data, then selected dependencies are loaded by absolute path.
 // We do not change the process/system PATH or rely on LoadLibrary's legacy search.
 split(directories_,environment(L"PATH"));

}
RuntimeDependencies::~RuntimeDependencies(){for(auto i=modules_.rbegin();i!=modules_.rend();++i)FreeLibrary(*i);for(auto c:cookies_)RemoveDllDirectory(c);}
std::filesystem::path RuntimeDependencies::find(const wchar_t* name)const{for(const auto& dir:directories_){std::error_code ec;auto path=dir/name;if(std::filesystem::is_regular_file(path,ec))return path;}return {};}
std::vector<LibraryDiagnostic> RuntimeDependencies::inspect(){
 diagnostics_.clear();for(auto name:required){LibraryDiagnostic d;d.name=std::filesystem::path(name).u8string();auto path=find(name);if(path.empty()){d.state="missing";d.message="Not found in runtime/gpu, APEX_CUDA_DLL_DIRS, CUDA_PATH/bin, CUDNN_PATH/bin or PATH";}else{d.path=path.u8string();d.state=x64(path)?"found_x64":"wrong_architecture";d.message=d.state=="found_x64"?"Located; loading and GPU execution not yet verified":"Expected a Windows x64 PE DLL";}diagnostics_.push_back(d);}return diagnostics_;
}
void RuntimeDependencies::load(){
 inspect();for(const auto& d:diagnostics_)if(d.state!="found_x64")throw std::runtime_error(d.name+": "+d.state+". Run setup-gpu-runtime.ps1 -InstallDependencies or supply compatible x64 CUDA 12/cuDNN 9 directories.");
 std::vector<std::filesystem::path> selected;for(const auto& d:diagnostics_)add(selected,std::filesystem::u8path(d.path).parent_path());
 for(const auto& dir:selected){auto cookie=AddDllDirectory(dir.c_str());if(cookie)cookies_.push_back(cookie);}
 for(auto name:{L"nvJitLink_120_0.dll",L"nvrtc64_120_0.dll"}){auto path=find(name);if(!path.empty()){auto cookie=AddDllDirectory(path.parent_path().c_str());if(cookie)cookies_.push_back(cookie);auto module=LoadLibraryExW(path.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);if(module)modules_.push_back(module);}}
 // Keep only the selected dependency directories registered for dependent loads;
 // core modules are explicitly loaded in a deterministic order from full paths.
 for(auto& d:diagnostics_){auto file=std::filesystem::u8path(d.path);HMODULE existing=GetModuleHandleW(file.filename().c_str());if(existing){auto loaded=loadedModulePath(existing);std::error_code ec;if(!std::filesystem::equivalent(std::filesystem::u8path(loaded),file,ec)){d.state="module_conflict";d.message="A different DLL with this name is already loaded: "+loaded+". Restart with a consistent runtime folder.";throw std::runtime_error(d.name+": "+d.message);}}
  HMODULE module=LoadLibraryExW(file.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);if(!module){d.errorCode=GetLastError();d.state="load_failed";d.message=windowsLoadError(d.errorCode)+" (the named file exists; a transitive dependency/version can still be missing)";throw std::runtime_error(d.name+": "+d.message);}modules_.push_back(module);d.state="loaded";d.path=loadedModulePath(module);d.message="DLL loaded; not proof of CUDA inference";
 }
}
}
