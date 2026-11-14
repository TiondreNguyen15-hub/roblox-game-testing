#include "apex/enemy.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <cmath>
using namespace apex;
void require(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
int main(){try{CapturedFrame f;f.width=832;f.height=416;f.rgba.assign(832*416*4,255);auto im=prepareEnemyImage(f);require(im.nchw.size()==3*416*416&&im.left==0&&im.top==104&&std::abs(im.scale-.5f)<1e-6,"Letterbox geometry wrong");require(im.nchw[0]>0.44f&&im.nchw[104*416]==1,"Padding or normalization wrong");std::vector<float> data(1800);data[0]=104;data[1]=104;data[2]=312;data[3]=312;data[4]=.9f;auto boxes=decodeEnemyOutput(data.data(),data.size(),im,.5f,.45f);require(boxes.size()==1&&std::abs(boxes[0].x1-.25f)<1e-5&&boxes[0].y1==0&&boxes[0].y2==1,"Inverse letterbox coordinates wrong");std::copy_n(data.data(),6,data.data()+6);require(decodeEnemyOutput(data.data(),data.size(),im,.5f,.45f).size()==1,"Duplicate boxes not suppressed");require(decodeEnemyOutput(data.data(),data.size(),im,.95f,.45f).empty(),"Threshold failed");data[5]=1;bool rejected=false;try{decodeEnemyOutput(data.data(),data.size(),im,.5f,.45f);}catch(...){rejected=true;}require(rejected,"Unexpected class guessed as enemy");std::cout<<"PASS RGB letterbox, original-frame mapping, actual score threshold, NMS and single-class contract\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
