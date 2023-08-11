#include "m_juno.h"

static int juno_getVersion(lua_State* L)
{
    lua_pushstring(L, JUNO_VERSION);
    return 1;
}

static const luaL_Reg reg[] = {
    { "getVersion", juno_getVersion },
    { NULL, NULL }
};

static const luaL_Reg mods[] = {
    /* Objects */
    { "Font", luaopen_font },
    { "Buffer", luaopen_buffer },
    { "Source", luaopen_source },
    { "Data", luaopen_data },
    { "Gif", luaopen_gif },
    { "Joystick", luaopen_joystick_object },
    /* Modules */
    { "window", luaopen_window },
    { "event", luaopen_event },
    { "system", luaopen_system },
    { "filesystem", luaopen_filesystem },
    { "timer", luaopen_timer },
    { "graphics", luaopen_graphics },
    { "audio", luaopen_audio },
    { "mouse", luaopen_mouse },
    { "joystick", luaopen_joystick },
    { NULL, NULL }
};

int luaopen_juno(lua_State* L)
{
    luaL_newlib(L, reg);

    for(int i = 0; mods[i].name; i++)
    {
        mods[i].func(L);
        lua_setfield(L, -2, mods[i].name);
    }

    return 1;
}