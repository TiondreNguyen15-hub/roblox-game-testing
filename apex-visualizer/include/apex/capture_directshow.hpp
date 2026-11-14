#pragma once
#include "capture.hpp"
#include <functional>
namespace apex {
// Caller owns a COM MTA. Only an explicitly OBS-named virtual camera is opened.
struct CameraInventory{std::vector<std::string> names;bool obsFound=false;};
bool isObsVirtualCamera(const std::string& name);
CameraInventory directShowCameras();
// Integration probe uses a real recording, never a simulated camera registration.
void probeDirectShowRecording(const std::filesystem::path&,std::atomic<bool>& stop,const std::function<void(std::shared_ptr<CapturedFrame>)>& publish);
void runObsDirectShow(std::atomic<bool>& stop,const std::function<void(std::shared_ptr<CapturedFrame>)>& publish,const std::function<void(const std::string&)>& status);
}
