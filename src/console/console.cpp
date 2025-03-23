#include "console.hpp"
#include "../core/core.hpp"
#include "../fonts/fonts.hpp"
#include "../levels/levels.hpp"
#include "../player/player.hpp"
#include <GL/gl.h>
#include <stdio.h>

#include <string>
#include <vector>

#include <SDL2/SDL_mixer.h>

bool                     showConsole = false;
std::string              inputCommand;
std::vector<std::string> history;
int                      historyIndex = -1;

// function to process user input commands from the Developer Console
void processCommand(const std::string &command) {
    // check if the command matches predefined functions and perform corresponding actions
    if (command == "clear") {
        history.clear(); // clear console history
    } else if (command == "exit") {
        std::exit(0); // terminate the Engine
    } else if (command == "ignorepos") {
        freezeCameraPos = !freezeCameraPos; // toggle freezing camera position
    } else if (command == "ignoreyaw") {
        freezeYaw = !freezeYaw; // toggle freezing yaw rotation
    } else if (command == "ignoreinput") {
        freezeCameraPos = !freezeCameraPos; // toggle freezing camera position and yaw rotation
    } else if (command == "relevel" && load_levels(level_path) == 0) {
        history.push_back(std::string("Loaded level: ") + level_path); // load a new level and log the result in console history
    } else if (command == "resxpos") {
        camerapos = og_camerapos; // reset camera position to original value
    } else if (command == "resxyaw") {
        yaw = og_yaw; // reset yaw angle to original value
    } else if (command == "resx_posyaw") {
        camerapos = og_camerapos,
        yaw       = og_yaw; // reset camera position and yaw angle to original values
    } else if (command == "close") {
        showConsole = false; // hide the console interface
    } else if (command == "stopsong") {
        Mix_HaltMusic(); // stop playing music
    }
    /* Load a music file and play it using SDL_mixer library */
    /*
    Mix_Music *music = Mix_LoadMUS(song_level);
    if (!music)
    {
        fprintf(stderr, "Failed to load music: %s\n", Mix_GetError());
        cleanup_resources(window, context, nullptr);
        return EXIT_FAILURE;
    }
    Mix_PlayMusic(music, -1);
    */
    else if (command == "wlevel") {
        history.push_back("Level name is: " + std::string(level_path)); // log the current level in console history
    } else if (command.rfind("load", 0) == 0) {
        // load a specified level and log the result in console history
        std::string level_name = command.substr(5);
        if (level_name.empty())
            history.push_back("Error: No level name specified for loading.");
        else {
            try {
                history.push_back(load_levels(level_name.c_str()) == 0 ? "Loaded level: " + level_name : "[WARNING] Failed load level file: " + level_name);
            } catch (const std::exception &e) {
                history.push_back("[ERROR] Error loading level: " + std::string(e.what()));
            } catch (...) {
                history.push_back("[ERROR] Unexpected error while loading level");
            }
        }
    } else {
        history.push_back("Unknown command: " + command);
    }
}

// function to render the console interface
void renderConsole() {
    static float currentHeight = 0.0f;    // current height of the console
    const float  targetHeight  = 200.0f;  // desired height of the console
    static float textOffset    = 0.0f;    // offset for text positioning within the console
    const float  slideSpeed    = 1300.0f; // speed at which the console slides (pixels per second)

    // animation function to smoothly animate the height of the console
    if (currentHeight < targetHeight) {
        float heightDelta = slideSpeed * delta_time;
        currentHeight += heightDelta;
        textOffset = currentHeight - targetHeight; // text starts from the bottom of the console

        if (currentHeight > targetHeight) {
            currentHeight = targetHeight;
            textOffset    = 0.0f;
        }
    }

    // prepare OpenGL environment for rendering
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // render the console's background
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);                        // top-left corner of the console
    glVertex2f(SCREEN_WIDTH, 0);             // top-right corner of the console
    glVertex2f(SCREEN_WIDTH, currentHeight); // bottom-right corner of the console
    glVertex2f(0, currentHeight);            // bottom-left corner of the console
    glEnd();

    // only render text if the console is mostly visible
    if (currentHeight > 20.0f) {
        glColor4f(1, 1, 1, 1); // white color for text

        int y = 10 + textOffset; // starting position for rendered text
        for (int i = history.size() - 1; i >= 0 && y < currentHeight - 20; i--) {
            renderText(history[i].c_str(), 10, y); // render each line of console history
            y += 20;                               // move to the next position for the next line
        }
        std::string line = "> " + inputCommand; // construct command input line
        renderText(line.c_str(), 10, y);        // render the command input line
    }
}
