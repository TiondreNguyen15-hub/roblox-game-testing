#include "apex/log.hpp"
#include "apex/config.hpp"
#include <mutex>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <ctime>
namespace apex {
std::filesystem::path Log::path(){return Config::defaultPath().parent_path()/"apex.log";}
void Log::write(const std::string& level,const std::string& message)noexcept{try{writeTo(path(),level,message);}catch(...){}}
void Log::writeTo(const std::filesystem::path& file,const std::string& level,const std::string& message)noexcept{
 try{static std::mutex lock;std::lock_guard<std::mutex> guard(lock);if(!file.parent_path().empty())std::filesystem::create_directories(file.parent_path());
 if(std::filesystem::exists(file)&&std::filesystem::file_size(file)>1024*1024){auto old=file;old+=".1";std::error_code ec;std::filesystem::remove(old,ec);std::filesystem::rename(file,old,ec);}
 auto now=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());std::tm utc{};
#ifdef _WIN32
 gmtime_s(&utc,&now);
#else
 gmtime_r(&now,&utc);
#endif
 std::ofstream out(file,std::ios::app);out<<std::put_time(&utc,"%Y-%m-%dT%H:%M:%SZ")<<" ["<<level<<"] ";
 for(char c:message)out<<(c=='\r'||c=='\n'?' ':c);
 out<<'\n';
 }catch(...){} // Logging must not terminate error recovery.
}
}
