#include "graphics.hpp"
#include <algorithm> // for std::min and std::max (btw)
#include <cmath>

// draws a vertical wall slice with texture Levelsping and shading
void draw_wall_slice(int screenX, int wallHeight, float shade, GLuint texture,
                     float rayDirX, float rayDirY, float fuckDistance) {
    if (wallHeight <= 0 || screenX < 0 || screenX >= SCREEN_WIDTH || texture == 0)
        return;

    int screenCenter = SCREEN_HEIGHT / 2;
    int wallTop      = screenCenter - (wallHeight / 2);
    int wallBottom   = screenCenter + (wallHeight / 2);

    // calculate hit coordinates and derive a texture coordinate
    float hitX         = camerapos.x + rayDirX * fuckDistance;
    float hitY         = camerapos.y + rayDirY * fuckDistance;
    float texX         = hitX - std::floor(hitX);
    float texY         = hitY - std::floor(hitY);
    float textureCoord = std::fmod(texX + texY, 1.0f);

    // combine the given shade with ambient lighting and clamp between 0 and 1
    float finalShade = std::max(0.0f, std::min(1.0f, shade + ambient_light));

    glBindTexture(GL_TEXTURE_2D, texture);
    glColor3f(finalShade, finalShade, finalShade);

    glBegin(GL_QUADS);
    const float texSliceWidth = 0.01f; // width of the texture slice
    float       xPos          = static_cast<float>(screenX);

    glTexCoord2f(textureCoord, 0.0f);
    glVertex2f(xPos, wallTop);

    glTexCoord2f(textureCoord + texSliceWidth, 0.0f);
    glVertex2f(xPos + 1.0f, wallTop);

    glTexCoord2f(textureCoord + texSliceWidth, 1.0f);
    glVertex2f(xPos + 1.0f, wallBottom);

    glTexCoord2f(textureCoord, 1.0f);
    glVertex2f(xPos, wallBottom);
    glEnd();
}

// renders the 3D scene using a raycasting approach
void render_scene() {
    float startAngle = yaw - (FOV / 2.0f);
    float angleStep  = FOV / SCREEN_WIDTH; // NUM_RAYS was just SCREEN_WIDTH

    GLuint wallTexture = load_texture(wallsTexture);

    // enable texture Levelsping once at the start
    glEnable(GL_TEXTURE_2D);

    // NUM_RAYS was just SCREEN_WIDTH
    for (int rayIndex = 0; rayIndex < SCREEN_WIDTH; ++rayIndex) {
        float rayAngle = startAngle + rayIndex * angleStep;
        float rayDirX  = std::cos(rayAngle);
        float rayDirY  = std::sin(rayAngle);

        float t            = 0.0f;
        bool  fucked       = false;
        float fuckDistance = 0.0f;

        // ray marching until a wall is hit or maximum distance is reached
        while (!fucked && t < 20.0f) {
            t += 0.01f;
            int LevelsX = static_cast<int>(camerapos.x + rayDirX * t);
            int LevelsY = static_cast<int>(camerapos.y + rayDirY * t);

            if (LevelsX >= 0 && LevelsX < level_WIDTH && LevelsY >= 0 && LevelsY < level_HEIGHT) {
                if (level[LevelsY][LevelsX] == 1) {
                    fucked       = true;
                    fuckDistance = t;
                }
            } else {
                // out of bounds; exit the loop to avoid infinite iteration
                fucked       = true;
                fuckDistance = t;
            }
        }

        if (fucked) {
            // correct distance to prevent fisheye effect
            // FIXME: didnt work
            float correctedDistance = fuckDistance * std::cos(rayAngle - yaw);
            int   wallHeight        = static_cast<int>(SCREEN_HEIGHT / correctedDistance);
            float shade             = 1.0f / (1.0f + correctedDistance * 0.1f);
            draw_wall_slice(rayIndex, wallHeight, shade, wallTexture, rayDirX, rayDirY, fuckDistance);
        }
    }

    glDisable(GL_TEXTURE_2D);
}

// simple skybox render for inital on project =D
// EDIT 23/03/25: no fuck you
void render_skybox() {
    GLuint sky = load_texture(skybox); // load the skybox

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, sky);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(0, 0);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(SCREEN_WIDTH, 0);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT / 2);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(0, SCREEN_HEIGHT / 2);

    glEnd();

    glDisable(GL_TEXTURE_2D);
}
