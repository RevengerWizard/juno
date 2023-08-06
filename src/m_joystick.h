#ifndef M_JOYSTICK_H
#define M_JOYSTICK_H

#include <SDL.h>

#define JOYSTICK_CLASS_NAME "Joystick"

typedef struct
{
    SDL_Joystick* joystick;
} Joystick;

#endif