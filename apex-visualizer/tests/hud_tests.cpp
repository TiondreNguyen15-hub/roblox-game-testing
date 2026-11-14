#include "apex/hud.hpp"
#include <iostream>
#include <fstream>
#include <limits>
#include <stdexcept>
using namespace apex;
void require(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
int main(){auto base=std::filesystem::temp_directory_path()/"apex-hud-tests";try{
 for(const auto& input:{std::string("28 / 160"),std::string("28/160\n"),std::string(" 28 / 160 ")}){auto v=interpretHudText(input,HudMode::Ammo);require(v.magazine==28&&v.reserve==160&&v.status=="ammo_candidate","Slash ammo parsing failed");}
 auto zero=interpretHudText("0",HudMode::Ammo);require(zero.magazine&&*zero.magazine==0&&!zero.reserve,"Zero counter hidden");
 for(const auto& input:{"2B","28 160","HP 100","1234","28 /","28/160 HP","=2+2"})require(!interpretHudText(input,HudMode::Ammo).magazine,"Ambiguous text guessed as ammo");
 require(interpretHudText("YOU ARE THE CHAMPION",HudMode::Result).banner=="champion","Banner parse failed");require(interpretHudText("NOT SQUAD ELIMINATED",HudMode::Result).banner.empty(),"Partial banner guessed");require(interpretHudText("Ammo 28",HudMode::Text).raw=="Ammo 28","Raw text lost");
 CapturedFrame f;f.width=4;f.height=4;f.rgba.resize(64);for(size_t i=0;i<16;++i){f.rgba[i*4]=static_cast<unsigned char>(i);f.rgba[i*4+3]=255;}HudOptions o;o.scale=2;o.region={.5f,.5f,.5f,.5f};auto crop=cropHudFrame(f,o);require(crop.width==4&&crop.height==4&&crop.rgba[0]==10&&crop.rgba[12]==11,"ROI mapping/upscale wrong");
 o.region.x=std::numeric_limits<float>::quiet_NaN();require(!validHudOptions(o),"NaN ROI accepted");o=HudOptions{};std::string error;std::filesystem::create_directories(base);require(saveHudOptions(base/"hud.json",o,error),"HUD save failed");HudOptions loaded;require(loadHudOptions(base/"hud.json",loaded,error)&&loaded.scale==o.scale,"HUD load failed");std::ofstream(base/"hud.json")<<"{\"version\":99}";loaded.scale=3;require(!loadHudOptions(base/"hud.json",loaded,error)&&loaded.scale==3,"Bad settings replaced working setup");
 auto row=interpretHudText("=formula,\"test\"\nline",HudMode::Text);row.source="recording.mp4";writeHudCsv(base/"out.csv",{row});std::ifstream in(base/"out.csv",std::ios::binary);std::string csv((std::istreambuf_iterator<char>(in)),{});require(csv.find("\"'=formula,")!=std::string::npos&&csv.find("\"\"test\"\"")!=std::string::npos,"CSV escaping unsafe");
 in.close();std::filesystem::remove_all(base);std::cout<<"PASS: conservative HUD parsing, exact ROI coordinates, scale, settings preservation and CSV escaping\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';std::error_code ec;std::filesystem::remove_all(base,ec);return 1;}}
