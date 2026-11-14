#include "apex/hud.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <stdexcept>
namespace apex {
bool validHudOptions(const HudOptions& o){const auto& r=o.region;return std::isfinite(r.x)&&std::isfinite(r.y)&&std::isfinite(r.width)&&std::isfinite(r.height)&&r.x>=0&&r.y>=0&&r.width>0&&r.height>0&&r.x+r.width<=1.000001f&&r.y+r.height<=1.000001f&&o.scale>=1&&o.scale<=4&&o.intervalMs>=250&&o.intervalMs<=10000&&static_cast<int>(o.mode)>=0&&static_cast<int>(o.mode)<=2;}
HudImage cropHudFrame(const CapturedFrame& f,const HudOptions& o){
 if(!validHudOptions(o)||!f.width||!f.height||f.width>4096||f.height>2160||f.rgba.size()!=size_t(f.width)*f.height*4)throw std::runtime_error("Invalid HUD region or captured frame");
 unsigned x=std::min(f.width-1,static_cast<unsigned>(o.region.x*f.width)),y=std::min(f.height-1,static_cast<unsigned>(o.region.y*f.height));unsigned right=std::clamp(static_cast<unsigned>(std::ceil((o.region.x+o.region.width)*f.width)),x+1,f.width),bottom=std::clamp(static_cast<unsigned>(std::ceil((o.region.y+o.region.height)*f.height)),y+1,f.height);
 unsigned w=right-x,h=bottom-y;double scale=std::min(double(o.scale),std::min(1600./w,1600./h));HudImage out;out.width=std::max(1u,static_cast<unsigned>(std::round(w*scale)));out.height=std::max(1u,static_cast<unsigned>(std::round(h*scale)));out.rgba.resize(size_t(out.width)*out.height*4);
 for(unsigned row=0;row<out.height;++row)for(unsigned col=0;col<out.width;++col){size_t in=(size_t(y+std::min(h-1,static_cast<unsigned>(row/scale)))*f.width+x+std::min(w-1,static_cast<unsigned>(col/scale)))*4;size_t at=(size_t(row)*out.width+col)*4;std::copy_n(f.rgba.data()+in,4,out.rgba.data()+at);}return out;
}
HudReading interpretHudText(const std::string& raw,HudMode mode){
 HudReading r;r.raw=raw.substr(0,4096);r.mode=mode;if(raw.empty()){r.status="no_text";return r;}if(mode==HudMode::Text){r.status="text_read";return r;}
 std::string text;bool space=false;for(unsigned char c:raw){if(c==' '||c=='\t'||c=='\r'||c=='\n'){space=!text.empty();continue;}if(space){text+=' ';space=false;}text+=static_cast<char>(c>='a'&&c<='z'?c-'a'+'A':c);}
 if(mode==HudMode::Result){if(text=="YOU ARE THE CHAMPION"||text=="CHAMPION"){r.banner="champion";r.status="banner_candidate";}else if(text=="SQUAD ELIMINATED"){r.banner="eliminated";r.status="banner_candidate";}else r.status="unrecognized_banner";return r;}
 // Only isolated decimal counters (or magazine / reserve pairs). No O->0,
 // fuzzy text repair, numeric guessing or confidence percentages.
 size_t at=0;auto spaces=[&]{while(at<text.size()&&text[at]==' ')++at;};auto number=[&](unsigned maxDigits)->std::optional<unsigned>{spaces();size_t begin=at;unsigned value=0;while(at<text.size()&&text[at]>='0'&&text[at]<='9'){value=value*10+unsigned(text[at++]-'0');if(at-begin>maxDigits)return {};}if(at==begin)return {};return value;};
 auto mag=number(3);if(!mag){r.status="ambiguous_ammo";return r;}spaces();std::optional<unsigned> reserve;if(at<text.size()&&text[at]=='/'){++at;reserve=number(4);if(!reserve){r.status="ambiguous_ammo";return r;}spaces();}if(at!=text.size()){r.status="ambiguous_ammo";return r;}r.magazine=mag;r.reserve=reserve;r.status="ammo_candidate";return r;
}
bool saveHudOptions(const std::filesystem::path& file,const HudOptions& o,std::string& error){try{if(!validHudOptions(o))throw std::runtime_error("Invalid HUD setup");nlohmann::json j={{"version",1},{"x",o.region.x},{"y",o.region.y},{"width",o.region.width},{"height",o.region.height},{"mode",static_cast<int>(o.mode)},{"scale",o.scale},{"interval_ms",o.intervalMs}};std::filesystem::create_directories(file.parent_path());std::ofstream f(file);f<<j.dump(2);f.flush();if(!f)throw std::runtime_error("HUD setup write failed");error.clear();return true;}catch(const std::exception& e){error=e.what();return false;}}
bool loadHudOptions(const std::filesystem::path& file,HudOptions& o,std::string& error){try{if(std::filesystem::file_size(file)>16384)throw std::runtime_error("HUD settings exceed 16 KiB");std::ifstream f(file);nlohmann::json j;f>>j;if(j.at("version")!=1)throw std::runtime_error("Unsupported HUD setup version");HudOptions c;c.region={j.at("x"),j.at("y"),j.at("width"),j.at("height")};c.mode=static_cast<HudMode>(j.at("mode").get<int>());c.scale=j.at("scale");c.intervalMs=j.at("interval_ms");if(!validHudOptions(c))throw std::runtime_error("Invalid HUD setup");o=c;error.clear();return true;}catch(const std::exception& e){error=e.what();return false;}}
void writeHudCsv(const std::filesystem::path& path,const std::vector<HudReading>& readings){
 auto field=[](std::string text){if(!text.empty()&&(text[0]=='='||text[0]=='+'||text[0]=='-'||text[0]=='@'||text[0]=='\t'||text[0]=='\r'))text="'"+text;std::string escaped="\"";for(char c:text){if(c=='\"')escaped+='\"';escaped+=c;}return escaped+'\"';};
 std::ofstream f(path,std::ios::binary);if(!f)throw std::runtime_error("Cannot open HUD CSV output");f<<"source,frame_sequence,source_timestamp_100ns,ocr_latency_ms,mode,status,magazine_candidate,reserve_candidate,banner_candidate,region_x,region_y,region_width,region_height,raw_text\r\n";
 for(const auto& r:readings){f<<field(r.source)<<','<<r.sequence<<','<<r.timestamp100ns<<','<<r.latencyMs<<','<<static_cast<int>(r.mode)<<','<<field(r.status)<<',';if(r.magazine)f<<*r.magazine;f<<',';if(r.reserve)f<<*r.reserve;f<<','<<field(r.banner)<<','<<r.region.x<<','<<r.region.y<<','<<r.region.width<<','<<r.region.height<<','<<field(r.raw)<<"\r\n";}f.flush();if(!f)throw std::runtime_error("HUD CSV write failed");
}
}
