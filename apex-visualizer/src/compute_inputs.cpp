#include "apex/compute.hpp"
#include "bundled_model.hpp"
#include <nlohmann/json.hpp>
#include <cctype>
#include <cstring>
#include <limits>
namespace apex {
std::filesystem::path normalizeComputePath(const std::filesystem::path& path,const std::filesystem::path& base){
 auto text=path.u8string();auto trim=[&]{auto first=text.find_first_not_of(" \t\r\n");if(first==std::string::npos){text.clear();return;}auto last=text.find_last_not_of(" \t\r\n");text=text.substr(first,last-first+1);};trim();
 if(text.size()>=2&&((text.front()=='"'&&text.back()=='"')||(text.front()=='\''&&text.back()=='\''))){text=text.substr(1,text.size()-2);trim();}
 if(text.empty())return {};auto result=std::filesystem::u8path(text);if(result.is_relative())result=base/result;return result.lexically_normal();
}
static std::vector<unsigned char> modelBytes(){
 static_assert(sizeof(float)==4&&std::numeric_limits<float>::is_iec559,"Validation asset requires IEEE FP32");std::vector<unsigned char> out(std::begin(modelPrefix),std::end(modelPrefix));out.reserve(262392);
 for(float v:computeWeights()){std::uint32_t bits=0;std::memcpy(&bits,&v,4);for(unsigned shift=0;shift<32;shift+=8)out.push_back(static_cast<unsigned char>(bits>>shift));}
 out.insert(out.end(),std::begin(modelSuffix),std::end(modelSuffix));return out;
}
bool prepareComputeInputs(ComputeOptions& o){
 o.modelPath=normalizeComputePath(o.modelPath,o.baseDirectory);o.inputProfile=normalizeComputePath(o.inputProfile,o.baseDirectory);const bool useDefault=o.modelPath.empty();if(useDefault)o.modelPath=o.baseDirectory/"models/compute_validation.onnx";
 bool restored=false;std::error_code ec;bool exists=std::filesystem::exists(o.modelPath,ec);if(ec)throw ComputeInputError("MODEL_PATH_UNREADABLE","Cannot inspect model path: "+o.modelPath.u8string()+" / "+ec.message());
 if(!exists&&useDefault){auto temp=o.modelPath;temp+=std::string(".restore-")+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());try{
   std::filesystem::create_directories(o.modelPath.parent_path());auto bytes=modelBytes();std::ofstream file(temp,std::ios::binary|std::ios::trunc);file.write(reinterpret_cast<const char*>(bytes.data()),static_cast<std::streamsize>(bytes.size()));file.close();if(!file)throw std::runtime_error("Could not write recovered model");
   // A concurrently created model is never replaced.
   if(std::filesystem::exists(o.modelPath))std::filesystem::remove(temp);else {std::filesystem::rename(temp,o.modelPath);restored=true;}
  }catch(const std::exception& e){std::filesystem::remove(temp,ec);throw ComputeInputError("BUNDLED_MODEL_RESTORE_FAILED","Cannot restore bundled model at "+o.modelPath.u8string()+": "+e.what()+". Extract the complete update into a writable app folder.");}
 }
 ec.clear();if(!std::filesystem::is_regular_file(o.modelPath,ec))throw ComputeInputError("MODEL_FILE_MISSING","ONNX model file not found: "+o.modelPath.u8string()+". Use bundled test model, or select an existing .onnx file.");
 if(!std::ifstream(o.modelPath,std::ios::binary))throw ComputeInputError("MODEL_PATH_UNREADABLE","Cannot read ONNX model: "+o.modelPath.u8string());
 if(!o.inputProfile.empty()){
  ec.clear();if(!std::filesystem::is_regular_file(o.inputProfile,ec))throw ComputeInputError("INPUT_PROFILE_MISSING","Input shape JSON not found: "+o.inputProfile.u8string()+". Leave this field blank for the bundled test model.");
  try{if(std::filesystem::file_size(o.inputProfile)>65536)throw std::runtime_error("Limit is 64 KiB");std::ifstream file(o.inputProfile);nlohmann::json j;file>>j;if(!j.is_object())throw std::runtime_error("Expected an object mapping input names to dimensions");}
  catch(const std::exception& e){throw ComputeInputError("INPUT_PROFILE_INVALID","Invalid input shape JSON: "+o.inputProfile.u8string()+" / "+e.what());}
 }
 return restored;
}
void markComputeStartupFailure(ComputeSnapshot& s,const std::string& code,const std::string& message){
 s.phase="unavailable_or_failed";s.error=message;s.providerStatus="Compute failed ["+code+"]: "+message;
 for(auto* test:{&s.cuda,&s.cpu,&s.gpu})if(test->state=="not_run"){test->state="blocked";test->reason=code;test->blockedBy="preflight";test->message=message;}
}
}
