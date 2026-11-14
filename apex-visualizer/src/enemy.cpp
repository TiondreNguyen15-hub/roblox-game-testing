#include "apex/enemy.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <stdexcept>
namespace apex {
EnemyImage prepareEnemyImage(const CapturedFrame& f){
 if(!f.width||!f.height||f.width>4096||f.height>2160||f.rgba.size()!=size_t(f.width)*f.height*4)throw std::runtime_error("Invalid enemy-detector frame");EnemyImage out;out.width=f.width;out.height=f.height;out.scale=std::min(416.f/f.width,416.f/f.height);unsigned w=std::clamp(static_cast<unsigned>(std::round(f.width*out.scale)),1u,416u),h=std::clamp(static_cast<unsigned>(std::round(f.height*out.scale)),1u,416u);out.left=(416-w)/2;out.top=(416-h)/2;out.nchw.assign(3*416*416,114.f/255.f);
 for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x){float sx=std::clamp((x+.5f)*f.width/w-.5f,0.f,float(f.width-1)),sy=std::clamp((y+.5f)*f.height/h-.5f,0.f,float(f.height-1));unsigned x0=static_cast<unsigned>(sx),y0=static_cast<unsigned>(sy),x1=std::min(x0+1,f.width-1),y1=std::min(y0+1,f.height-1);float dx=sx-x0,dy=sy-y0;for(unsigned c=0;c<3;++c){auto pixel=[&](unsigned a,unsigned b){return float(f.rgba[(size_t(b)*f.width+a)*4+c]);};float value=(pixel(x0,y0)*(1-dx)+pixel(x1,y0)*dx)*(1-dy)+(pixel(x0,y1)*(1-dx)+pixel(x1,y1)*dx)*dy;out.nchw[size_t(c)*416*416+size_t(y+out.top)*416+x+out.left]=value/255.f;}}
 return out;
}
std::vector<EnemyBox> decodeEnemyOutput(const float* data,size_t count,const EnemyImage& image,float threshold,float overlap){
 if(!data||count!=1800||!std::isfinite(image.scale)||image.scale<=0||!image.width||!image.height||!std::isfinite(threshold)||threshold<0||threshold>1||!std::isfinite(overlap)||overlap<0||overlap>1)throw std::runtime_error("Invalid [1,300,6] enemy output/options");std::vector<EnemyBox> candidates;
 for(size_t i=0;i<count;i+=6){for(size_t c=0;c<6;++c)if(!std::isfinite(data[i+c]))throw std::runtime_error("Enemy model produced non-finite values");float score=data[i+4],id=data[i+5];if(score<0||score>1||std::abs(id)>1e-3f)throw std::runtime_error("Enemy model must output probability scores and class 0 only");if(score<threshold)continue;
  EnemyBox b{std::clamp((data[i]-image.left)/image.scale/image.width,0.f,1.f),std::clamp((data[i+1]-image.top)/image.scale/image.height,0.f,1.f),std::clamp((data[i+2]-image.left)/image.scale/image.width,0.f,1.f),std::clamp((data[i+3]-image.top)/image.scale/image.height,0.f,1.f),score};if(b.x2>b.x1&&b.y2>b.y1)candidates.push_back(b);
 }
 std::sort(candidates.begin(),candidates.end(),[](const auto& a,const auto& b){return a.score>b.score;});std::vector<EnemyBox> kept;for(const auto& b:candidates){bool suppress=false;for(const auto& a:kept){float intersection=std::max(0.f,std::min(a.x2,b.x2)-std::max(a.x1,b.x1))*std::max(0.f,std::min(a.y2,b.y2)-std::max(a.y1,b.y1));float sum=(a.x2-a.x1)*(a.y2-a.y1)+(b.x2-b.x1)*(b.y2-b.y1)-intersection;if(sum>0&&intersection/sum>overlap){suppress=true;break;}}if(!suppress)kept.push_back(b);}return kept;
}
bool saveEnemyOptions(const std::filesystem::path& path,const EnemyOptions& o,std::string& error){try{nlohmann::json j={{"version",1},{"model",o.modelPath.u8string()},{"prefer_cuda",o.preferCuda},{"confidence",o.confidence},{"iou",o.iou},{"interval_ms",o.intervalMs}};std::filesystem::create_directories(path.parent_path());std::ofstream f(path);f<<j.dump(2);f.flush();if(!f)throw std::runtime_error("Enemy setup write failed");error.clear();return true;}catch(const std::exception& e){error=e.what();return false;}}
bool loadEnemyOptions(const std::filesystem::path& path,EnemyOptions& o,std::string& error){try{if(std::filesystem::file_size(path)>16384)throw std::runtime_error("Enemy setup too large");nlohmann::json j;std::ifstream(path)>>j;if(j.at("version")!=1)throw std::runtime_error("Unknown enemy setup version");EnemyOptions next=o;next.modelPath=std::filesystem::u8path(j.at("model").get<std::string>());next.preferCuda=j.at("prefer_cuda");next.confidence=j.at("confidence");next.iou=j.at("iou");next.intervalMs=j.at("interval_ms");if(!std::isfinite(next.confidence)||next.confidence<0||next.confidence>1||!std::isfinite(next.iou)||next.iou<0||next.iou>1||next.intervalMs<50||next.intervalMs>5000)throw std::runtime_error("Invalid enemy setup values");o=next;error.clear();return true;}catch(const std::exception& e){error=e.what();return false;}}
}
