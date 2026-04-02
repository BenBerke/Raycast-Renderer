//
// Created by berke on 3/11/2026.
//

#ifndef RAYCAST_RENDERER_CONFIG_H
#define RAYCAST_RENDERER_CONFIG_H


#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 960

#define RAY_SIZE 0.1f
#define MAX_RAY_LENGTH 1500

#define FOV 60
#define RAY_COUNT 45

#define PLAYER_SPEED 350
#define PLAYER_ROT_SPEED 0.9f
#define PLAYER_FRICTION .3f
#define PLAYER_SCALE 15

#define WALL_HEIGHT 100
#define MAX_WALL_OVERLAP 4

#define FPS_CAP INFINITY
#define FRAME_DELAY (1000 / FPS_CAP)

#define AMBIENT 1.0f
#define SUN_DIRECTION 0 // Lights up: 0-North, 1-East, 2-South, 3-West
#define SUN_STRENGTH 0.7f

#endif //RAYCAST_RENDERER_CONFIG_H