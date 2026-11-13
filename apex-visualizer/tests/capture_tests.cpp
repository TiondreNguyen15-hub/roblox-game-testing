#include "apex/capture.hpp"
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>
int main(int argc,char** argv){try{
 if(argc!=2)throw std::runtime_error("Expected original generated recording fixture path");
 apex::CaptureSource source;source.start(std::filesystem::u8path(argv[1]));auto start=std::chrono::steady_clock::now();apex::CaptureSnapshot final;double activity=0;std::uint64_t last=0;
 do{std::this_thread::sleep_for(std::chrono::milliseconds(5));final=source.snapshot();if(final.frame&&last!=final.frame->sequence){last=final.frame->sequence;activity+=final.statistics.meanAbsoluteChange;}if(!final.running)break;}while(std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count()<15);
 source.stop();if(final.state!="end_of_recording"||final.frames!=30||final.width!=320||final.height!=180||!final.frame||activity<=0)throw std::runtime_error("Real recording decode failed: "+final.state+" / "+final.message+" frames="+std::to_string(final.frames)+" size="+std::to_string(final.width)+"x"+std::to_string(final.height)+" activity="+std::to_string(activity)+" luma="+std::to_string(final.statistics.meanLuma));
 source.start(std::filesystem::u8path(argv[1]).parent_path()/"absent-test-recording.mp4");std::this_thread::sleep_for(std::chrono::milliseconds(300));source.stop();if(source.snapshot().state!="unavailable")throw std::runtime_error("Missing recording not rejected");
 source.start(std::filesystem::u8path(argv[1]));std::this_thread::sleep_for(std::chrono::milliseconds(150));source.stop();if(source.snapshot().running)throw std::runtime_error("Capture stop failed");
 std::cout<<"PASS: actual H264 Media Foundation decode: 30 real frames, moving pixel statistics, missing file recovery and stop/restart\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
