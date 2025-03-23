#include <SDL2/SDL_error.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

// SDL2 headers
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include <GL/gl.h>
#include <GL/glu.h>

// engine files
#include "../console/console.hpp"
#include "../fonts/fonts.hpp"
#include "../graphics/graphics.hpp"
#include "../levels/levels.hpp"
#include "../logsystem/log.hpp"
#include "../player/player.hpp"
#include "core.hpp"
#include "types.hpp"

// engine vars
// nothing now btw

// global vars
bool party = true;

// set the pointer to "nullptr"
extern TTF_Font *font;

// player and some things
const float FOV        = M_PI / 3.0f; // field of view
float       delta_time = 0.0f;        // do not remove bitch, i am warning you

// other Level var
char  wallsTexture[];
char  skybox[];
char  song_level[];
char  creator[256]; // needs to be manual 256
char  description[256]; // also needs to be manual 256
float ambient_light = 0.0f;

// cache textures
std::unordered_map<std::string, GLuint> textureCache;

// function to load textures from files
GLuint load_texture(const char *filename) {
    // copy the input filename into a clean variable, removing any trailing newline characters
    char clean_filename[256];
    strncpy(clean_filename, filename, sizeof(clean_filename) - 1);
    clean_filename[sizeof(clean_filename) - 1]      = '\0';
    clean_filename[strcspn(clean_filename, "\r\n")] = 0;

    // check if the texture has already been loaded in the cache
    auto it = textureCache.find(clean_filename);
    if (it != textureCache.end())
        return it->second; // return the cached texture ID if found

    // load the image from the given filename or a fallback image
    SDL_Surface *surface = IMG_Load(clean_filename);
    if (!surface) {
        error("Error loading", clean_filename);
        error("Using fallback texture...");
        surface = IMG_Load("resources/textures/missing.png"); // load a missing texture as fallback
    }

    if (!surface) {
        panic("Error loading fallback image", IMG_GetError());
        exit(EXIT_FAILURE);
    }

    if (GENERAL_DEBUG) {
        debug("Loading texture from", clean_filename);
    }

    // convert the surface format to the desired OpenGL format (RGBA or RGB) if necessary
    SDL_PixelFormatEnum target_format = (surface->format->BytesPerPixel == 4)
                                            ? SDL_PIXELFORMAT_RGBA32
                                            : SDL_PIXELFORMAT_RGB24;
    SDL_Surface        *converted     = SDL_ConvertSurfaceFormat(surface, target_format, 0);
    if (!converted) {
        error("Error converting surface format", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // define the OpenGL internal and data formats based on the converted surface format (RGBA or RGB)
    GLenum internal_format = (converted->format->BytesPerPixel == 4) ? GL_RGBA : GL_RGB;
    GLenum data_format     = internal_format;

    // generate a new OpenGL texture and bind it
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // set pixel storage alignment to 1 byte
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // upload the texture data to OpenGL and configure texture parameters (min/mag filtering, wrapping)
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, converted->w, converted->h, 0,
                 data_format, GL_UNSIGNED_BYTE, converted->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // set nearest filtering for minification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // set nearest filtering for magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);      // set texture wrapping to repeat in S direction
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);      // set texture wrapping to repeat in T direction

    // free the loaded and converted surfaces and store the texture ID in the cache for future use
    SDL_FreeSurface(converted);
    SDL_FreeSurface(surface);
    textureCache[clean_filename] = texture;
    return texture;
}

// check OpenGL for errors and output for the user
bool check_for_gl_error(const char *error_message) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "Error: %s - 0x%x\n", error_message, error);
        return true;
    }
    return false;
}

// initializes the OpenGL projection matrix
int setup_projection() {
    GLenum previous_error = glGetError(); // stores any existing GL errors
    if (previous_error != GL_NO_ERROR)    // checks if there are any existing GL errors
    {
        fprintf(stderr, "Warning: Existing GL error: 0x%x\n", previous_error);
    }

    glMatrixMode(GL_PROJECTION);                                   // switches to the projection matrix mode
    if (check_for_gl_error("Switching to projection matrix mode")) // checks for GL errors
        return 0;

    glLoadIdentity();                                  // loads identity matrix on the current matrix stack
    if (check_for_gl_error("Loading identity matrix")) // checks for GL errors
        return 0;

    glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0);     // sets an orthographic projection with the specified parameters
    if (check_for_gl_error("Setting orthographic projection")) // checks for GL errors
        return 0;

    glMatrixMode(GL_MODELVIEW);                                    // switches to the model view matrix mode
    if (check_for_gl_error("Switching to model view matrix mode")) // Checks for GL errors
        return 0;

    glLoadIdentity();                                      // resets the model view matrix
    if (check_for_gl_error("Resetting model view matrix")) // checks for GL errors
        return 0;

    return 1;
}

// function to check for collisions during gameplay
bool check_collision(int new_x, int new_y) {
    // verifies that the new position does not exceed the level boundaries and is a walkable area
    if (new_x < 0 || new_y < 0 || new_x >= level_WIDTH || new_y >= level_HEIGHT)
        return false;
    // checks whether the level at the new position contains a walkable tile (1)
    return level[new_y][new_x] == 1;
}

// function to initialize SDL and its subsystems (Video, Audio, and Image).
// also initializes the TrueType Font (SDL_ttf) library.
int initialize_sdl_and_subsystems() {
    // initialize the Video subsystem of SDL.
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        panic("Failed to initialize SDL", SDL_GetError());
    }

    // initialize the Audio subsystem of SDL.
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        panic("Failed to initialize audio", Mix_GetError());
    }

    // initialize the Image subsystem of SDL (for loading PNG images).
    if (IMG_Init(IMG_INIT_PNG) < 0) {
        panic("Failed to initialize SDL Image", IMG_GetError());
    }

    // initialize the TrueType Font (SDL_ttf) library.
    if (TTF_Init() == -1) {
        panic("Error initializing SDL_ttf", TTF_GetError());
    }

    // load a font for console and gameplay from the "resources/fonts/font.ttf" file.
    font = TTF_OpenFont(defaultFontPath, 16);
    if (!font) {
        panic("Error initializing font", TTF_GetError());
    }

    return 1;
}

void cleanup_resources(SDL_Window *window, SDL_GLContext context, Mix_Music *music) {
    if (music)
        Mix_FreeMusic(music);
    if (context)
        SDL_GL_DeleteContext(context);
    if (window)
        SDL_DestroyWindow(window);
    Mix_CloseAudio();
    SDL_Quit();
}

int main() {
    if (!initialize_sdl_and_subsystems())
        return EXIT_FAILURE;

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, ANTI_ALIASING);

    SDL_Window *window = SDL_CreateWindow(SCREEN_TITLE,
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT,
                                          SDL_WINDOW_OPENGL);

    SDL_SetRelativeMouseMode(SDL_TRUE);

    if (!window) {
        cleanup_resources(nullptr, nullptr, nullptr);
        panic("Window creation failed", SDL_GetError());
    }

    SDL_Surface *icon_surface = IMG_Load("logo/window.png");
    if (!icon_surface) {
        cleanup_resources(window, nullptr, nullptr);
        panic("Failed to load window icon", IMG_GetError());
    }
    SDL_SetWindowIcon(window, icon_surface);
    SDL_FreeSurface(icon_surface);

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        cleanup_resources(window, nullptr, nullptr);
        panic("OpenGL context creation failed", SDL_GetError());
    }
    glEnable(GL_MULTISAMPLE);

    if (GENERAL_DEBUG) {
        debug("Loading default level...");
    }
    if (load_levels(defaultLevelsPath) != 0) {
        cleanup_resources(window, context, nullptr);
        return EXIT_FAILURE;
    }

    Mix_Music *music = Mix_LoadMUS(song_level);
    if (!music) {
        error("Failed to load music", Mix_GetError());
        free(music);
    }
    Mix_PlayMusic(music, -1);

    setup_projection();

    float TARGET_FPS  = 60.0f;
    float FRAME_DELAY = 1000.0f / TARGET_FPS;

    const Uint8 *state           = SDL_GetKeyboardState(nullptr);
    bool         running         = true;
    Uint32       last_frame_time = SDL_GetTicks();

    SDL_StartTextInput();

    while (running) {
        Uint32 frame_start = SDL_GetTicks();

        // process events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_F1) {
                    showConsole = !showConsole;
                    if (showConsole)
                        SDL_StartTextInput();
                    else
                        SDL_StopTextInput();
                }
                if (showConsole) {
                    if (event.key.keysym.sym == SDLK_RETURN) {
                        history.push_back(inputCommand);
                        processCommand(inputCommand);
                        inputCommand.clear();
                    } else if (event.key.keysym.sym == SDLK_BACKSPACE && !inputCommand.empty()) {
                        inputCommand.pop_back();
                    }
                }
            } else if (event.type == SDL_TEXTINPUT && showConsole) {
                inputCommand += event.text.text;
            }
        }

        // update delta time
        Uint32 current_frame = SDL_GetTicks();
        delta_time           = (current_frame - last_frame_time) / 1000.0f;
        last_frame_time      = current_frame;

        // movement handling (only when console is hidden)
        float move_x = 0.0f,
              move_y = 0.0f;
        if (!showConsole) {
            handle_movement(state, move_x, move_y, yaw, delta_time);
        }
        float new_x = camerapos.x + move_x * BASE_PLAYER_SPEED;
        float new_y = camerapos.y + move_y * BASE_PLAYER_SPEED;
        if (!check_collision(new_x, new_y)) {
            camerapos.x = new_x;
            camerapos.y = new_y;
        }

        // debug info
        if (CAMERA_DEBUG) {
            printf("Camera: pos(%.2f, %.2f, %.2f), yaw: %.2f\n",
                   camerapos.x, camerapos.y, camerapos.z, yaw);
        }

        // clamp camera z
        if (camerapos.z > 1.0f)
            camerapos.z = 1.0f;
        else if (camerapos.z < -1.0f)
            camerapos.z = -1.0f;

        // render scene
        glClear(GL_COLOR_BUFFER_BIT);
        render_skybox();
        render_scene();
        if (showConsole) {
            renderConsole();
        }
        SDL_GL_SwapWindow(window);

        // frame rate control
        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frame_time);
        }
    }

    cleanup_resources(window, context, music);
    return 0;
}
