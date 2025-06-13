#include <stdio.h>

// headers
#include "core.hpp"

const size_t PATH_MAX_LENGTH = 256;

// defines super importants constants for the engine
const int   SCREEN_WIDTH                       = 800;
const int   SCREEN_HEIGHT                      = 600;
const char  SCREEN_TITLE[]                     = "Vortox (fuck you legacy code)";
const float ANTI_ALIASING                      = 8.0f;
const char  defaultLevelsPath[]                = "levels/d1.xym";
const char  defaultFontPath[]                  = "resources/fonts/font.ttf";

// debug
const bool G_debug = true;
const bool G_cameraDebug = false;

// player
const bool  ENABLE_LEGACY_TURNING = false;