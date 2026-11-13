#include "apex/log.hpp"
#include <fstream>
#include <chrono>
#include <iostream>
#include <stdexcept>
int main(){auto dir=std::filesystem::temp_directory_path()/("apex-log-test-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));try{
 auto file=dir/"app.log";apex::Log::writeTo(file,"INFO","one\ntwo");std::ifstream in(file);std::string line;std::getline(in,line);if(line.find("[INFO] one two")==std::string::npos)throw std::runtime_error("Log formatting/sanitization failed");in.close();
 {std::ofstream f(file,std::ios::app);f<<std::string(1100000,'x');}apex::Log::writeTo(file,"WARN","rotated");if(!std::filesystem::exists(dir/"app.log.1")||std::filesystem::file_size(file)>1024)throw std::runtime_error("Log rotation failed");
 {std::ofstream f(dir/"not-a-directory");f<<"x";}apex::Log::writeTo(dir/"not-a-directory"/"app.log","ERROR","Must not throw");
 std::filesystem::remove_all(dir);std::cout<<"PASS: logging, newline sanitization, bounded rotation and unwritable destination recovery\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';std::filesystem::remove_all(dir);return 1;}}
