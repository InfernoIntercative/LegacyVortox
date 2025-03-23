#include <cmath>
#include <stdio.h>

// headers
#include "player.hpp"
#include "../core/types.hpp"
#include "../core/core.hpp"

// base speeds
const float BASE_PLAYER_SPEED = 1.5f;
const float BASE_TURN_SPEED   = 3.14159f * 4.0f;
const float MOUSE_SENSITIVITY = 0.1f / 256.0f;

// constant for normalization
constexpr float EPSILON = 1e-6;

// global camera state
Vector3 camerapos       = {5.0f, 5.0f, 0.0f};
Vector3 og_camerapos    = camerapos;
float   yaw             = 0.0f;
float   og_yaw          = yaw;
bool    freezeCameraPos = false;
bool    freezeYaw       = false;

float TEMP_FIX_FOR_LEGACY_TURNING = 8.0f;

/**
 * @brief Handles the player's movement based on keyboard input.
 *
 * @param state Pointer to the keyboard state array.
 * @param move_x Reference for horizontal movement.
 * @param move_y Reference for vertical movement.
 * @param yaw Reference for the yaw (rotation).
 * @param delta_time Time elapsed since the last frame (in seconds).
 */
void handle_movement(const Uint8 *state, float &move_x, float &move_y, float &yaw, float delta_time) {
    // compute current speeds using delta_time
    float playerSpeed = BASE_PLAYER_SPEED * delta_time;
    float turnSpeed   = BASE_TURN_SPEED * delta_time;

    move_x = 0.0f;
    move_y = 0.0f;

    // precompute cos and sin of yaw for movement
    float cos_yaw = cos(yaw);
    float sin_yaw = sin(yaw);

    if (!freezeCameraPos) {
        // Forward/Backward
        if (state[SDL_SCANCODE_W]) {
            move_x += cos_yaw * playerSpeed;
            move_y += sin_yaw * playerSpeed;
        }
        if (state[SDL_SCANCODE_S]) {
            move_x -= cos_yaw * playerSpeed;
            move_y -= sin_yaw * playerSpeed;
        }

        // strafe movement
        float strafe_yaw     = yaw + M_PI / 2.0f;
        float cos_strafe_yaw = cos(strafe_yaw);
        float sin_strafe_yaw = sin(strafe_yaw);

        if (state[SDL_SCANCODE_A]) {
            move_x -= cos_strafe_yaw * playerSpeed;
            move_y -= sin_strafe_yaw * playerSpeed;
        }
        if (state[SDL_SCANCODE_D]) {
            move_x += cos_strafe_yaw * playerSpeed;
            move_y += sin_strafe_yaw * playerSpeed;
        }

        // optional normalization (ensures consistent speed when moving diagonally)
        float length = sqrt(move_x * move_x + move_y * move_y);
        if (length > EPSILON) {
            move_x = (move_x / length) * playerSpeed;
            move_y = (move_y / length) * playerSpeed;
        }
    }

    if (!freezeYaw) {
        if (!ENABLE_LEGACY_TURNING) {
            int mouseXRel, mouseYRel;
            SDL_GetRelativeMouseState(&mouseXRel, &mouseYRel);
            // apply turning using both BASE_TURN_SPEED and MOUSE_SENSITIVITY
            yaw += mouseXRel * turnSpeed * MOUSE_SENSITIVITY;
        } else {
            if (state[SDL_SCANCODE_LEFT]) {
                yaw -= turnSpeed / TEMP_FIX_FOR_LEGACY_TURNING; // FIXME: needs delta_time, but delta_time is 0.0f
                // i prob will never fix this =)
            }
            if (state[SDL_SCANCODE_RIGHT]) {
                yaw += turnSpeed / TEMP_FIX_FOR_LEGACY_TURNING; // FIXME: needs delta_time, but delta_time is 0.0f
                // i prob will never fix this =)
            }
        }
    }
}
