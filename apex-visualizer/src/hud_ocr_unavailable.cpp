#include "apex/hud.hpp"
#include <stdexcept>
namespace apex {std::unique_ptr<HudTextReader> makeHudTextReader(){throw std::runtime_error("This cross-build has no Windows OCR projection. Use the native Visual Studio Windows build.");}}
