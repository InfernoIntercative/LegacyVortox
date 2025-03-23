#include "fonts.hpp"
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

TTF_Font *font = nullptr;

void renderText(const char *text, int x, int y) {
    SDL_Color    color   = {255, 255, 255, 255};
    SDL_Surface *surface = TTF_RenderText_Blended(font, text, color);
    if (!surface) // if tere was an error rendering the text
    {
        printf("Error rendering text: %s\n", TTF_GetError());
        return;
    }

    // convert the surface to RGBA32 format
    SDL_Surface *formatted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    if (!formatted) // if there was an error converting the surface format
    {
        printf("Error converting surface format: %s\n", SDL_GetError());
        return;
    }

    int w = formatted->w;
    int h = formatted->h;

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, formatted->pixels);

    SDL_FreeSurface(formatted);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(x, y);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(x + w, y);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(x + w, y + h);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(x, y + h);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDeleteTextures(1, &texture);
}
