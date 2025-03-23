#pragma once

#include "../core/types.hpp"
#include <SDL2/SDL.h>

// define the player's speed (adjust as needed)
extern float       delta_time;
extern const float BASE_PLAYER_SPEED;
extern const float BASE_TURN_SPEED;
extern const float MOUSE_SENSITIVITY;

// player's movement angles (yaw and yaw)
extern float yaw; // horizontal angle

// important
extern Vector3 camerapos;
extern Vector3 og_camerapos;
extern float   yaw;
extern float   og_yaw;
extern bool    freezeCameraPos;
extern bool    freezeYaw;

// function to handle player movement based on key states
void handle_movement(const Uint8 *state, float &move_x, float &move_y, float &yaw, float delta_time);
