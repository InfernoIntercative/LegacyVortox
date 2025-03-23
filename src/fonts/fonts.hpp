#pragma once

#include <stdio.h>
#include <SDL2/SDL_ttf.h>

extern TTF_Font *font;

void renderText(const char *text, int x, int y);
