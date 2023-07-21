#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif
#include <stdbool.h>

#include <SDL.h>

#include "luax.h"

typedef struct
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
} SDLWrapper;

extern SDLWrapper* sdlwrap;

static bool fullscreen = false;

static int window_setTitle(lua_State* L)
{
    const char* title = luaL_checkstring(L, 1);
    SDL_SetWindowTitle(sdlwrap->window, title);
    return 0;
}

static int window_getTitle(lua_State* L)
{
    const char* title = SDL_GetWindowTitle(sdlwrap->window);
    lua_pushstring(L, title);
    return 1;
}

static int window_setSize(lua_State* L)
{
    SDL_SetWindowSize(sdlwrap->window, luaL_checknumber(L, 1), luaL_checknumber(L, 2));
    return 0;
}

static int graphics_setFullscreen(lua_State* L)
{
    fullscreen = luax_optboolean(L, 1, 0);
    SDL_SetWindowFullscreen(sdlwrap->window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    return 0;
}

static const luaL_Reg reg[] = {
    { "setSize", window_setSize },
    { "setTitle", window_setTitle },
    { "getTitle", window_getTitle },
    { "setFullscreen", graphics_setFullscreen },
    {NULL, NULL}
};

int luaopen_window(lua_State* L)
{
    luaL_newlib(L, reg);
    return 1;
}
