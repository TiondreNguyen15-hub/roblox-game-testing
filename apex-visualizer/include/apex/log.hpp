#pragma once
#include <filesystem>
#include <string>
namespace apex {
class Log {public:static std::filesystem::path path();static void writeTo(const std::filesystem::path&,const std::string& level,const std::string& message) noexcept;static void write(const std::string& level,const std::string& message) noexcept;};
}
