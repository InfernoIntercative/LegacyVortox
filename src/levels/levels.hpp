#pragma once

#include <cstddef>

extern int   level_WIDTH;
extern int   level_HEIGHT;
extern int **level;

extern const size_t PATH_MAX_LENGTH;

extern char song_level[256];
extern char wallsTexture[256];
extern char skybox[256];

extern float ambient_light;
extern char  level_path[256];

int load_levels(const char *filename);
