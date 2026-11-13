#pragma once
#include "compute.hpp"
#include <windows.h>
#include <filesystem>
#include <vector>
namespace apex {
// Owns explicit directory registrations and module references for one validation run.
class RuntimeDependencies {
public:
 explicit RuntimeDependencies(std::filesystem::path runtimeFolder);~RuntimeDependencies();
 RuntimeDependencies(const RuntimeDependencies&)=delete;RuntimeDependencies& operator=(const RuntimeDependencies&)=delete;
 std::vector<LibraryDiagnostic> inspect();
 void load();const std::vector<LibraryDiagnostic>& diagnostics()const{return diagnostics_;}
 const std::vector<std::filesystem::path>& directories()const{return directories_;}
private:
 std::filesystem::path find(const wchar_t* name)const;
 std::vector<std::filesystem::path> directories_;std::vector<DLL_DIRECTORY_COOKIE> cookies_;std::vector<HMODULE> modules_;std::vector<LibraryDiagnostic> diagnostics_;
};
std::string loadedModulePath(HMODULE module);
std::string windowsLoadError(unsigned long code);
}
