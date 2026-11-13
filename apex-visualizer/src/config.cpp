#include "apex/config.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#ifdef _WIN32
#include <windows.h>
#endif
namespace apex {
std::filesystem::path Config::defaultPath(){
#ifdef _WIN32
 wchar_t buffer[32768];DWORD n=GetEnvironmentVariableW(L"LOCALAPPDATA",buffer,32768);
 if(n&&n<32768)return std::filesystem::path(buffer)/L"ApexVisualization"/L"settings.cfg";
#endif
 return std::filesystem::current_path()/"apex-settings.cfg";
}
bool Config::save(const std::filesystem::path& path,const Settings& input,std::string& error){
 try{
  Settings s=input;s.clamp();if(!path.parent_path().empty())std::filesystem::create_directories(path.parent_path());
  auto temp=path;temp+=".tmp";std::ofstream out(temp,std::ios::trunc);if(!out)throw std::runtime_error("Cannot open temporary configuration");
  out<<std::setprecision(9)<<"version=1\n";
#define WRITE(k) out<<#k<<'='<<s.k<<'\n'
  WRITE(boxes);WRITE(corners);WRITE(skeleton);WRITE(health);WRITE(distance);WRITE(names);WRITE(teams);WRITE(visibility);WRITE(enemiesOnly);WRITE(visibleOnly);WRITE(vsync);WRITE(animate);WRITE(maxDistance);WRITE(menuWidth);WRITE(menuHeight);WRITE(entityCount);WRITE(frameLimit);
#undef WRITE
  for(int i=0;i<3;++i)out<<"accent"<<i<<'='<<s.accent[i]<<'\n';
  out.flush();if(!out)throw std::runtime_error("Configuration write failed");out.close();if(!out)throw std::runtime_error("Configuration close failed");
#ifdef _WIN32
  if(!MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("Atomic configuration replacement failed");
#else
  std::filesystem::rename(temp,path);
#endif
  error.clear();return true;
 }catch(const std::exception& e){error=e.what();return false;}
}
bool Config::load(const std::filesystem::path& path,Settings& output,std::string& error){
 try{
  std::ifstream in(path);if(!in)throw std::runtime_error("Configuration does not exist or cannot be read");
  Settings next;std::string line;bool version=false;int lines=0;
  while(std::getline(in,line)){
   if(++lines>128||line.size()>256)throw std::runtime_error("Configuration too large");
   if(line.empty()||line[0]=='#')continue;
   auto split=line.find('=');if(split==std::string::npos)throw std::runtime_error("Malformed configuration line");
   auto key=line.substr(0,split),raw=line.substr(split+1);std::istringstream parse(raw);double v;parse>>v;
   if(!parse||!std::isfinite(v))throw std::runtime_error("Invalid numeric setting");
   parse>>std::ws;if(!parse.eof())throw std::runtime_error("Trailing configuration data");
   if(key=="version"){if(v!=1)throw std::runtime_error("Unsupported configuration version");version=true;continue;}
#define BOOL(k) if(key==#k){if(v!=0&&v!=1)throw std::runtime_error("Invalid boolean");next.k=v!=0;continue;}
   BOOL(boxes)BOOL(corners)BOOL(skeleton)BOOL(health)BOOL(distance)BOOL(names)BOOL(teams)BOOL(visibility)BOOL(enemiesOnly)BOOL(visibleOnly)BOOL(vsync)BOOL(animate)
#undef BOOL
#define FLOAT(k) if(key==#k){next.k=static_cast<float>(v);continue;}
   FLOAT(maxDistance)FLOAT(menuWidth)FLOAT(menuHeight)
#undef FLOAT
   if(key=="entityCount"||key=="frameLimit"){if(v<0||v>10000||v!=std::floor(v))throw std::runtime_error("Invalid integer");(key=="entityCount"?next.entityCount:next.frameLimit)=static_cast<int>(v);continue;}
   if(key.size()==7&&key.compare(0,6,"accent")==0&&key[6]>='0'&&key[6]<='2')next.accent[key[6]-'0']=static_cast<float>(v);
  }
  if(!in.eof()||!version)throw std::runtime_error("Incomplete configuration");
  next.clamp();output=next;error.clear();return true;
 }catch(const std::exception& e){error=e.what();return false;}
}
}
