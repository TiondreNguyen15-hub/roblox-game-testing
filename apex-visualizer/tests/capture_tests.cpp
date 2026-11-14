#include "apex/capture.hpp"
#include "apex/capture_directshow.hpp"
#include <windows.h>
#include <objbase.h>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>
int main(int argc,char** argv){try{
 if(argc!=2)throw std::runtime_error("Expected original generated recording fixture path");
 apex::CaptureSource source;source.start(std::filesystem::u8path(argv[1]));auto start=std::chrono::steady_clock::now();apex::CaptureSnapshot final;double activity=0;std::uint64_t last=0;
 do{std::this_thread::sleep_for(std::chrono::milliseconds(5));final=source.snapshot();if(final.frame&&last!=final.frame->sequence){last=final.frame->sequence;activity+=final.statistics.meanAbsoluteChange;}if(!final.running)break;}while(std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count()<15);
 source.stop();if(final.state!="end_of_recording"||final.frames!=30||final.width!=320||final.height!=180||!final.frame||activity<=0)throw std::runtime_error("Real recording decode failed: "+final.state+" / "+final.message+" frames="+std::to_string(final.frames)+" size="+std::to_string(final.width)+"x"+std::to_string(final.height)+" activity="+std::to_string(activity)+" luma="+std::to_string(final.statistics.meanLuma));
 source.start(std::filesystem::u8path(argv[1]).parent_path()/"absent-test-recording.mp4");for(int i=0;i<500&&source.snapshot().running;++i)std::this_thread::sleep_for(std::chrono::milliseconds(10));source.stop();if(source.snapshot().state!="unavailable")throw std::runtime_error("Missing recording not rejected");
 source.start(std::filesystem::u8path(argv[1]));for(int i=0;i<500&&source.snapshot().frames<2;++i)std::this_thread::sleep_for(std::chrono::milliseconds(10));source.setPaused(true);std::this_thread::sleep_for(std::chrono::milliseconds(100));auto pausedFrame=source.snapshot().frames;std::this_thread::sleep_for(std::chrono::milliseconds(250));if(!source.snapshot().paused||!pausedFrame||source.snapshot().frames!=pausedFrame)throw std::runtime_error("Recording did not pause for HUD selection");source.setPaused(false);std::this_thread::sleep_for(std::chrono::milliseconds(300));if(source.snapshot().frames<=pausedFrame)throw std::runtime_error("Recording did not resume");source.stop();if(source.snapshot().running)throw std::runtime_error("Capture stop failed");
 if(!apex::isObsVirtualCamera("OBS Virtual Camera")||!apex::isObsVirtualCamera("obs VIRTUAL camera")||apex::isObsVirtualCamera("USB Camera")||apex::isObsVirtualCamera("Virtual Camera"))throw std::runtime_error("OBS-only device selection failed");
 auto apartment=CoInitializeEx(nullptr,COINIT_MULTITHREADED);if(FAILED(apartment))throw std::runtime_error("Test COM startup failed");struct Apartment{~Apartment(){CoUninitialize();}} apartmentCleanup;
 auto cameras=apex::directShowCameras();std::cout<<"DirectShow camera enumeration succeeded; count="<<cameras.names.size()<<"; OBS="<<cameras.obsFound<<'\n';
 if(!cameras.obsFound){source.start();for(int i=0;i<1500&&source.snapshot().running;++i)std::this_thread::sleep_for(std::chrono::milliseconds(10));source.stop();auto unavailable=source.snapshot();if(unavailable.state!="unavailable"||unavailable.message.find("DirectShow")==std::string::npos)throw std::runtime_error("Missing OBS did not produce DirectShow diagnostics: "+unavailable.message);std::cout<<"PASS: missing OBS rejected with both-backend diagnostics\n";}
 auto fixture=std::filesystem::u8path(argv[1]);fixture.replace_extension(".avi");std::atomic<bool> stop{false};bool received=false;
 apex::probeDirectShowRecording(fixture,stop,[&](std::shared_ptr<apex::CapturedFrame> frame){auto pixels=apex::frameStatistics(frame->rgba,frame->width,frame->height);if(frame->width!=160||frame->height!=90||pixels.meanLuma<=.05)throw std::runtime_error("Actual DirectShow RGB32 frame decoding failed");received=true;stop=true;});
 if(!received)throw std::runtime_error("No actual DirectShow video frame received");std::cout<<"PASS: actual AVI -> DirectShow graph -> RGB32 callback -> RGBA conversion, bounded frame and teardown\n";
 std::cout<<"PASS: actual H264 Media Foundation decode: 30 real frames, moving pixel statistics, missing file recovery and stop/restart\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
