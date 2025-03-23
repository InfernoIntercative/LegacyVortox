#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// headers
#include "levels.hpp"
#include "../logsystem/log.hpp"

int **level        = NULL;
int   level_WIDTH  = 0;
int   level_HEIGHT = 0;

/*
must match the header, if there is a warning, there is history
*/
extern char  song_level[256];
extern char  creator[256];
extern char  description[256];
extern char  skybox[256];
extern float ambient_light;
extern char  wallsTexture[256];
char         level_path[256] = {0};

int load_levels(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        warn("Failed to open level file", filename);
        return 1;
    }
    strncpy(level_path, filename, sizeof(level_path) - 1);
    level_path[sizeof(level_path) - 1] = '\0';

    char  line[256];
    float level_version     = 0;
    float supported_version = 4.21;
    /*
    initializing an array to a single null character is more efficient than initializing it with all zeros.
    */
    char branch[9]     = {'\0'};
    int  level_section = 0;

    int temp_Levels_width = 0;
    int temp_level_height = 0;
    int current_row       = 0;

    // first pass: determine dimensions and metadata
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        if (strstr(line, "level_version=") != NULL) {
            sscanf(line, "level_version =%f", &level_version);
            if (level_version != supported_version) {
                printf("Unsupported level version: %f\n", level_version);
                fclose(file);
                return -1;
            }
        } else if (strstr(line, "branch=") != NULL) {
            sscanf(line, "branch=%s", branch);
        } else if (strstr(line, "level_song=") != NULL) {
            sscanf(line, "    level_song= %*c%[^\"]%*c", song_level);
        } else if (strstr(line, "creator=") != NULL) {
            sscanf(line, "    creator= %s", creator);
        } else if (strstr(line, "description=") != NULL) {
            sscanf(line, "    description= %*c%[^\"]%*c", description);
        } else if (strstr(line, "ambient_light=") != NULL) {
            sscanf(line, "    ambient_light= %f", &ambient_light);
        } else if (strstr(line, "[LEVELDATA]") != NULL) {
            level_section = 1;
            continue;
        } else if (strstr(line, "[TEXTURES]") != NULL) {
            break;
        }

        if (level_section && strlen(line) > 0 && line[0] != '[') {
            // use a temporary copy so strtok doesn't alter the original line.
            char tempLine[256];
            strcpy(tempLine, line);
            if (temp_Levels_width == 0) {
                char *token = strtok(tempLine, " ");
                while (token != NULL) {
                    temp_Levels_width++;
                    token = strtok(NULL, " ");
                }
            }
            temp_level_height++;
        }
    }

    if (temp_level_height == 0 || temp_Levels_width == 0) {
        printf("Invalid level dimensions.\n");
        fclose(file);
        return -1;
    }

    // save to global level_WIDTH and level_HEIGHT
    level_WIDTH  = temp_Levels_width;
    level_HEIGHT = temp_level_height;

    // allocate the level array
    level = (int **)malloc(level_HEIGHT * sizeof(int *));
    if (!level) {
        fclose(file);
        panic("Failed to allocate memory for level rows.");
        return -1;
    }
    for (int i = 0; i < level_HEIGHT; i++) {
        level[i] = (int *)malloc(level_WIDTH * sizeof(int));
        if (!level[i]) {
            fclose(file);
            panic("Failed to allocate memory for level columns.");
        }
    }

    // reset file pointer for second pass
    fseek(file, 0, SEEK_SET);
    level_section = 0;
    current_row   = 0;

    // second pass: load the actual level data
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        if (strstr(line, "[LEVELDATA]") != NULL) {
            level_section = 1;
            continue;
        } else if (strstr(line, "[TEXTURES]") != NULL) {
            break;
        }

        if (level_section && strlen(line) > 0 && line[0] != '[') {
            if (current_row >= level_HEIGHT)
                break;

            char *token = strtok(line, " ");
            int   col   = 0;
            while (token != NULL && col < level_WIDTH) {
                level[current_row][col] = atoi(token);
                token                   = strtok(NULL, " ");
                col++;
            }
            if (col != level_WIDTH) {
                warn("Row has unexpected number of columns.");
            }
            current_row++;
        }
    }

    // read texture info (after the [Textures] section)
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "skybox=") != NULL) {
            sscanf(line, "skybox=%255[^\n]", skybox);
            if (GENERAL_DEBUG) {
                printf("Loaded texture: %s\n", skybox);
            }
            continue;
        }
        if (strstr(line, "1=") != NULL) {
            char *equals = strchr(line, '=');
            if (equals != NULL) {
                equals++;
                while (*equals == ' ' || *equals == '\t')
                    equals++;

                size_t len = strlen(equals);
                while (len > 0 && (equals[len - 1] == '\n' || equals[len - 1] == '\r' || equals[len - 1] == ' ' || equals[len - 1] == '\t')) {
                    equals[--len] = '\0';
                }

                strncpy(wallsTexture, equals, 255);
                wallsTexture[255] = '\0';
                if (GENERAL_DEBUG) {
                    debug("Loaded texture", wallsTexture);
                }
            }
            continue;
        }
    }

    fclose(file);
    if (GENERAL_DEBUG) {
        printf("Level loaded successfully!\n\n");
    }

    if (strcmp(branch, "beta") == 0) {
        printf("╔════════════ WARNING ════════════╗\n");
        printf("║       Beta Level detected!        ║\n");
        printf("║      These can be unstable!     ║\n");
        printf("╚═════════════════════════════════╝\n\n");
        printf("Level Info:\n");
        printf("├─ Version: %.1f\n", level_version);
        printf("├─ Level Path: %s\n", filename);
        printf("├─ Branch: %s\n", branch);
        printf("├─ Creator: %s\n", creator);
        printf("├─ Song: %s\n", song_level);
        printf("├─ Description: %s\n", description);
        printf("└─ Ambient Light: %.1f\n\n", ambient_light);
    }

    return 0;
}
