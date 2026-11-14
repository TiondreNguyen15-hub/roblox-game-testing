#include "apex/performance.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace apex {
MemoryTrend assessMemoryTrend(const std::vector<MemorySample>& values,bool vram){
 MemoryTrend r;std::vector<MemorySample> good;for(const auto& p:values){double m=vram?p.vramMiB:p.privateMiB;if(std::isfinite(p.seconds)&&std::isfinite(m)&&m>=0)good.push_back(p);}
 if(good.size()<60)return r;
 double start=good.front().seconds;double finish=good.back().seconds;r.spanSeconds=finish-start;r.samples=static_cast<unsigned>(good.size());if(r.spanSeconds<240||good.size()<r.spanSeconds*1.4){r.message="Insufficient duration or valid 2 Hz sample coverage";return r;}
 double sx=0,sy=0,sxx=0,sxy=0,first=0,last=0;unsigned nf=0,nl=0;double bins[4]{};unsigned counts[4]{};
 for(const auto& p:good){double x=p.seconds-start,y=vram?p.vramMiB:p.privateMiB;sx+=x;sy+=y;sxx+=x*x;sxy+=x*y;if(x<=30){first+=y;++nf;}if(p.seconds>=finish-30){last+=y;++nl;}auto bin=std::min(3u,static_cast<unsigned>(x/r.spanSeconds*4));bins[bin]+=y;++counts[bin];}
 if(!nf||!nl)return r;
 r.growthMiB=last/nl-first/nf;double denom=good.size()*sxx-sx*sx;r.slopeMiBPerMinute=denom>0?(good.size()*sxy-sx*sy)/denom*60:0;
 bool rising=true;for(unsigned i=0;i<4;++i)if(!counts[i])rising=false;for(unsigned i=1;i<4&&rising;++i)if(bins[i]/counts[i]<bins[i-1]/counts[i-1]+1)rising=false;
 double threshold=std::max(64.,first/nf*.02);bool flagged=r.growthMiB>threshold&&r.slopeMiBPerMinute>8&&rising;
 r.state=flagged?"suspected_growth":"stable_observed";r.message=flagged?"Sustained growth flagged; investigate allocations. This is not a definitive leak diagnosis.":"No sustained-growth heuristic triggered in observed window; leak freedom is not proven.";return r;
}
void yuy2ToRgba(const std::vector<unsigned char>& input,unsigned width,unsigned height,std::vector<unsigned char>& output){
 if(!width||!height||width%2||width>4096||height>2160||input.size()!=size_t(width)*height*2||&input==&output)throw std::runtime_error("Invalid bounded native YUY2 frame");
 output.resize(size_t(width)*height*4);
 for(size_t i=0,j=0;i<input.size();i+=4,j+=8){int u=int(input[i+1])-128,v=int(input[i+3])-128;for(unsigned p=0;p<2;++p){int y=std::max(0,int(input[i+p*2])-16);auto index=j+p*4;output[index]=static_cast<unsigned char>(std::clamp((298*y+409*v+128)>>8,0,255));output[index+1]=static_cast<unsigned char>(std::clamp((298*y-100*u-208*v+128)>>8,0,255));output[index+2]=static_cast<unsigned char>(std::clamp((298*y+516*u+128)>>8,0,255));output[index+3]=255;}}
}
FrameStatistics frameStatistics(const std::vector<unsigned char>& p,unsigned w,unsigned h,const std::vector<unsigned char>* previous){
 if(!w||!h||w>4096||h>2160||p.size()!=size_t(w)*h*4)throw std::runtime_error("Invalid captured RGBA dimensions");
 FrameStatistics r;const bool compare=previous&&previous->size()==p.size();size_t n=0;
 // Bounded sampling measures content activity without semantic player detection.
 for(size_t i=0;i<p.size();i+=4*16){double y=(.2126*p[i]+.7152*p[i+1]+.0722*p[i+2])/255.;r.meanLuma+=y;if(compare)r.meanAbsoluteChange+=std::abs(y-(.2126*(*previous)[i]+.7152*(*previous)[i+1]+.0722*(*previous)[i+2])/255.);++n;}
 r.meanLuma/=n;if(compare)r.meanAbsoluteChange/=n;r.changeValid=compare;return r;
}
}
