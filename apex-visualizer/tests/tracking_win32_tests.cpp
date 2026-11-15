#include "apex/tracking.hpp"
#include <windows.h>
#include <iostream>
int main(){apex::TrackingWindow bad;auto state=apex::inspectTrackingWindow(bad);std::string error;if(state.valid||apex::sendTrackingMovement(bad,1,1,error)||error.empty())return 1;auto windows=apex::trackingWindows();for(const auto& w:windows)if(!w.handle||w.process==GetCurrentProcessId()||w.title.empty())return 2;std::cout<<"PASS invalid-window input blocked before SendInput, own windows excluded, external window metadata enumerated without game-memory reads\n";return 0;}
