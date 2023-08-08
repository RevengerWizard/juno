#include <SDL.h>

#include "luax.h"

typedef struct
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
} SDLWrapper;

extern SDLWrapper* sdlwrap;

static int l_mouse_setVisible(lua_State* L)
{
    SDL_ShowCursor(lua_toboolean(L, 1));
    return 0;
}

static int l_mouse_setPosition(lua_State* L)
{
    int x = luaL_checknumber(L, 1);
    int y = luaL_checknumber(L, 2);
    SDL_WarpMouseInWindow(sdlwrap->window, x, y);
    return 0;
}

static const luaL_Reg reg[] = {
    { "setVisible", l_mouse_setVisible },
    { "setPosition", l_mouse_setPosition },
    { NULL, NULL }
};

int luaopen_mouse(lua_State* L)
{
    luaL_newlib(L, reg);
    return 1;
}