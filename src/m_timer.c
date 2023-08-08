#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include <SDL.h>

#include "luax.h"

static int l_timer_getTime(lua_State* L)
{
    lua_pushnumber(L, SDL_GetTicks() / 1000.0);
    return 1;
}

static int l_timer_sleep(lua_State* L)
{
    SDL_Delay(luaL_checknumber(L, 1) * 1000.0);
    return 0;
}

static const luaL_Reg reg[] = {
    { "getTime", l_timer_getTime },
    { "sleep", l_timer_sleep },
    {NULL, NULL}
};

int luaopen_timer(lua_State* L)
{
    luaL_newlib(L, reg);
    return 1;
}
