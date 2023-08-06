#include <SDL.h>

#include "luax.h"
#include "m_joystick.h"

#define CLASS_NAME  JOYSTICK_CLASS_NAME

Joystick* joystick_new(lua_State* L)
{
    Joystick* self = (Joystick*)lua_newuserdata(L, sizeof(*self));
    luaL_setmetatable(L, CLASS_NAME);
    memset(self, 0, sizeof(*self));
    return self;
}

static int joystick_gc(lua_State* L)
{
    Joystick* self = (Joystick*)luaL_checkudata(L, 1, CLASS_NAME);
    SDL_JoystickClose(self->joystick);
    return 0;
}

static int joystick_getName(lua_State* L)
{
    Joystick* self = (Joystick*)luaL_checkudata(L, 1, CLASS_NAME);
    lua_pushstring(L, SDL_JoystickName(self->joystick));
    return 1;
}

static const luaL_Reg jreg[] = {
    { "__gc", joystick_gc },
    { "getName", joystick_getName },
    { NULL, NULL }
};

int luaopen_joystick_object(lua_State* L)
{
    luaL_newmetatable(L, CLASS_NAME);
    luaL_setfuncs(L, jreg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}

static int joystick_init(lua_State* L)
{
    if(SDL_Init(SDL_INIT_JOYSTICK) != 0)
    {
        luaL_error(L, "could not init joystick");
    }
    /* Make SDL push events through the event system */
    SDL_JoystickEventState(SDL_ENABLE);
    return 0;
}

static int joystick_open(lua_State *L) 
{
    int idx = luaL_checknumber(L, 1);
    Joystick* self;

    /* Try to avoid the SDL error */
    if(idx < 0 || idx >= SDL_NumJoysticks()) 
    {
        luaL_error(L, "Invalid joystick id: %d", idx);
    }
    self = joystick_new(L);
    self->joystick = SDL_JoystickOpen(idx);

    if(self->joystick == NULL) 
    {
        luaL_error(L, "Could not open game controller. Error: %s", SDL_GetError());
        return 0;
    }
    return 1;
}

static int joystick_getCount(lua_State* L)
{
    lua_pushnumber(L, SDL_NumJoysticks());
    return 1;
}

static const luaL_Reg reg[] = {
    { "init", joystick_init },
    { "open", joystick_open },
    { "getCount", joystick_getCount },
    { NULL, NULL }
};

int luaopen_joystick(lua_State* L)
{
    luaL_newlib(L, reg);
    return 1;
}