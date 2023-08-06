#ifndef M_JUNO_H
#define M_JUNO_H

#include "luax.h"

#define JUNO_VERSION "0.0.0"

int luaopen_source(lua_State* L);
int luaopen_data(lua_State* L);
int luaopen_buffer(lua_State* L);
int luaopen_font(lua_State* L);
int luaopen_joystick_object(lua_State* L);

int luaopen_joystick(lua_State* L);
int luaopen_audio(lua_State* L);
int luaopen_window(lua_State* L);
int luaopen_graphics(lua_State* L);
int luaopen_filesystem(lua_State* L);
int luaopen_mouse(lua_State* L);
int luaopen_event(lua_State* L);
int luaopen_system(lua_State* L);
int luaopen_timer(lua_State* L);

int luaopen_juno(lua_State* L);

#endif