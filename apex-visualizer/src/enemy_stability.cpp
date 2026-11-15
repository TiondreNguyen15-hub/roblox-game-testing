#include "apex/enemy.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace apex {namespace {
bool valid(const EnemyBox& b){return std::isfinite(b.x1)&&std::isfinite(b.y1)&&std::isfinite(b.x2)&&std::isfinite(b.y2)&&std::isfinite(b.score)&&b.x1>=0&&b.y1>=0&&b.x2<=1&&b.y2<=1&&b.x2>b.x1&&b.y2>b.y1&&b.score>=0&&b.score<=1;}
float overlap(const EnemyBox& a,const EnemyBox& b){float intersection=std::max(0.f,std::min(a.x2,b.x2)-std::max(a.x1,b.x1))*std::max(0.f,std::min(a.y2,b.y2)-std::max(a.y1,b.y1));float area=(a.x2-a.x1)*(a.y2-a.y1)+(b.x2-b.x1)*(b.y2-b.y1)-intersection;return area>0?intersection/area:0;}
}
void EnemyContinuity::reset(){confirmed_.clear();seen_=-1;width_=height_=0;}
std::vector<EnemyBox> EnemyContinuity::update(const std::vector<EnemyBox>& candidates,const EnemyOptions& options,const CapturedFrame& frame){double stamp=std::chrono::duration<double>(frame.capturedAt.time_since_epoch()).count();if(!std::isfinite(options.confidence)||options.confidence<0||options.confidence>1)throw std::runtime_error("Invalid continuity score threshold");if(!options.stabilize||width_!=frame.width||height_!=frame.height||stamp<seen_||stamp-seen_>.25)reset();width_=frame.width;height_=frame.height;std::vector<EnemyBox> kept;std::vector<bool> used(confirmed_.size());auto ordered=candidates;for(const auto& box:ordered)if(!valid(box))throw std::runtime_error("Invalid continuity candidate");std::stable_sort(ordered.begin(),ordered.end(),[](const auto& a,const auto& b){return a.score>b.score;});for(const auto& box:ordered){if(!valid(box))throw std::runtime_error("Invalid continuity candidate");bool accepted=box.score>=options.confidence;size_t match=confirmed_.size();float best=.3f;for(size_t n=0;n<confirmed_.size();++n)if(!used[n]){float score=overlap(box,confirmed_[n]);if(score>=best){match=n;best=score;}}if(!accepted&&options.stabilize&&box.score>=options.confidence*.7f&&match<confirmed_.size())accepted=true;if(accepted){kept.push_back(box);if(match<confirmed_.size())used[match]=true;}}
 if(!kept.empty()){confirmed_=kept;seen_=stamp;}return kept;
}
void EnemyDisplayCache::reset(){boxes_.clear();stamp_={};missing_={};source_.clear();width_=height_=0;completed_=0;}
EnemyDisplayState EnemyDisplayCache::update(const EnemySnapshot& snapshot,int maxAgeMs,int graceMs,bool enabled,std::chrono::steady_clock::time_point now){if(!enabled||snapshot.status=="unavailable"||!snapshot.completed||!snapshot.analyzedFrame||maxAgeMs<50||maxAgeMs>1000||graceMs<0||graceMs>300){reset();return {};}
 const auto& f=*snapshot.analyzedFrame;if(snapshot.completed<completed_||f.capturedAt<stamp_||f.capturedAt>now||source_!=snapshot.latest.source||width_!=f.width||height_!=f.height){reset();}completed_=snapshot.completed;source_=snapshot.latest.source;width_=f.width;height_=f.height;
 if(enemyResultFresh(snapshot,maxAgeMs,now)&&!snapshot.latest.boxes.empty()){boxes_=snapshot.latest.boxes;stamp_=f.capturedAt;missing_={};return {boxes_,false};}
 if(missing_.time_since_epoch().count()==0)missing_=now;
 if(!boxes_.empty()&&graceMs>0&&now>=stamp_&&now-stamp_<std::chrono::milliseconds(maxAgeMs+graceMs)&&now-missing_<std::chrono::milliseconds(graceMs))return {boxes_,true};boxes_.clear();return {};
}
}
