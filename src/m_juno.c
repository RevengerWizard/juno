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
    { "Source", luaopen_source },
    { "Data", luaopen_data },
    { "Buffer", luaopen_buffer },
    { "Font", luaopen_font },
    /*{ "Joystick", luaopen_joystick_object },*/
    /* Modules */
    { "joystick", luaopen_joystick },
    { "audio", luaopen_audio },
    { "window", luaopen_window },
    { "graphics", luaopen_graphics },
    { "system", luaopen_system },
    { "filesystem", luaopen_filesystem },
    { "mouse", luaopen_mouse },
    { "timer", luaopen_timer },
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