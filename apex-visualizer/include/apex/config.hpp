#pragma once
#include "settings.hpp"
#include <filesystem>
#include <string>
namespace apex {
class Config {
public:
 static std::filesystem::path defaultPath();
 static bool save(const std::filesystem::path&,const Settings&,std::string& error);
 static bool load(const std::filesystem::path&,Settings&,std::string& error);
};
}
