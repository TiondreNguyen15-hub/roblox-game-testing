#include "apex/compute.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
using namespace apex;
void require(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
std::vector<unsigned char> read(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
int main(int argc,char** argv){auto base=std::filesystem::temp_directory_path()/"apex model recovery test";try{
 if(argc!=2)throw std::runtime_error("Expected original model fixture");std::filesystem::remove_all(base);std::filesystem::create_directories(base);
 ComputeOptions o;o.baseDirectory=base;o.modelPath=" \"  \" ";o.inputProfile=" \t ";require(prepareComputeInputs(o),"Missing default not restored");require(read(o.modelPath)==read(std::filesystem::u8path(argv[1])),"Recovered ONNX bytes differ from original model");
 auto model=o.modelPath;ComputeOptions again;again.baseDirectory=base;require(!prepareComputeInputs(again),"Existing default replaced");
 o.modelPath=" \"models/compute_validation.onnx\" ";require(!prepareComputeInputs(o)&&o.modelPath==model,"Quoted relative model path failed");
 o.modelPath=base/"custom.onnx";bool rejected=false;try{prepareComputeInputs(o);}catch(const ComputeInputError& e){rejected=e.code=="MODEL_FILE_MISSING";}require(rejected&&!std::filesystem::exists(o.modelPath),"Missing custom model was not rejected without replacement");
 o.modelPath=model;o.inputProfile=" missing-shapes.json ";rejected=false;try{prepareComputeInputs(o);}catch(const ComputeInputError& e){rejected=e.code=="INPUT_PROFILE_MISSING";}require(rejected,"Missing input profile hidden");
 std::ofstream(base/"bad.json")<<"not json";o.inputProfile="\"bad.json\"";rejected=false;try{prepareComputeInputs(o);}catch(const ComputeInputError& e){rejected=e.code=="INPUT_PROFILE_INVALID";}require(rejected,"Malformed input profile accepted");
 ComputeSnapshot s;markComputeStartupFailure(s,"MODEL_FILE_MISSING","real missing path");require(s.cuda.state=="blocked"&&s.cpu.state=="blocked"&&s.gpu.state=="blocked"&&!s.gpu.verified&&s.providerStatus.find("real missing path")!=std::string::npos,"Startup failure was hidden or falsely verified");
 s.cpu.state="passed";s.cpu.verified=true;markComputeStartupFailure(s,"OTHER","later error");require(s.cpu.verified&&s.cpu.state=="passed","Existing verified result was removed");
 std::filesystem::remove_all(base);std::cout<<"PASS: exact bundled asset recovery, quoted/relative/empty paths, custom-file preservation, input profile diagnostics, honest blocked states\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';std::error_code ec;std::filesystem::remove_all(base,ec);return 1;}}
