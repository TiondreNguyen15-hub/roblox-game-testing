#include "apex/settings.hpp"
#include <algorithm>
#include <cmath>
namespace apex {
static float finiteClamp(float v,float lo,float hi,float fallback){return std::isfinite(v)?std::clamp(v,lo,hi):fallback;}
void Settings::clamp(){
 maxDistance=finiteClamp(maxDistance,10,300,160); menuWidth=finiteClamp(menuWidth,560,1200,710);menuHeight=finiteClamp(menuHeight,440,900,530);
 for(auto& v:accent)v=finiteClamp(v,0,1,.6f);
 playbackSpeed=finiteClamp(playbackSpeed,.1f,3.f,1.f);scene=std::clamp(scene,0,2);
 entityCount=std::clamp(entityCount,1,64);frameLimit=std::clamp(frameLimit,30,360);
}
bool applyJsonPath(Settings& settings,std::string text,std::string& error){
 const auto first=text.find_first_not_of(" \t\r\n");
 if(first==std::string::npos){error="Clipboard has no path.";return false;}
 text=text.substr(first,text.find_last_not_of(" \t\r\n")-first+1);
 if(text.size()>=2&&text.front()=='"'&&text.back()=='"')text=text.substr(1,text.size()-2);
 if(text.empty()||text.size()>=sizeof(settings.livePath)){error="Path must be 1-200 UTF-8 bytes; it was not truncated.";return false;}
 for(unsigned char c:text)if(c<32||c==127||c=='"'){error="Paste one file path, not multiple lines or a command.";return false;}
 std::copy(text.begin(),text.end(),settings.livePath);settings.livePath[text.size()]=0;error.clear();return true;
}

}
