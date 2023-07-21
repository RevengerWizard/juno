#ifndef LUAX_H
#define LUAX_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define luax_setfield_type(T, L, k, v) \
    do { lua_push##T(L, v); lua_setfield(L, -2, k); } while(0)

#define luax_setfield_number(L, k, v) luax_setfield_type(number, L, k, v)
#define luax_setfield_string(L, k, v) luax_setfield_type(string, L, k, v)
#define luax_setfield_boolean(L, k, v)   luax_setfield_type(boolean, L, k, v)
#define luax_setfield_udata(L, k, v)  luax_setfield_type(lightuserdata, L, k, v)
#define luax_setfield_cfunc(L, k, v)  luax_setfield_type(cfunction, L, k, v)
#define luax_setfield_fstring(L, k, ...) \
    do { lua_pushfstring(L, __VA_ARGS__); lua_setfield(L, -2, k); } while (0)

#define luax_optboolean(L, i, x) \
    (!lua_isnoneornil(L, i) ? lua_toboolean(L, i) : (x))

#if LUA_VERSION_NUM < 502

#define lua_absindex(L, i) (((i) > 0 || (i) < LUA_REGISTRYINDEX) ? (i) : lua_gettop(L)+(i)+1)

#define luaL_getsubtable luax_getsubtable

#define luaL_requiref(L, l, f, g) luax_requiref(L, (l), (f), (g))

int luax_getsubtable(lua_State *L, int idx, const char *fname);
void luax_requiref(lua_State* L, const char* modname, lua_CFunction openf, int glb);

#endif

#endif