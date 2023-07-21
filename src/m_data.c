#include <SDL.h>

#include "luax.h"
#include "m_data.h"
#include "fs.h"

#define CLASS_NAME  DATA_CLASS_NAME

static Data* new_data(lua_State* L)
{
    Data* self = (Data*)lua_newuserdata(L, sizeof(*self));
    luaL_setmetatable(L, CLASS_NAME);
    memset(self, 0, sizeof(*self));
    return self;
}

static int data_gc(lua_State* L)
{
    Data* self = (Data*)luaL_checkudata(L, 1, CLASS_NAME);
    free(self->data);
    return 1;
}

static int data_fromFile(lua_State* L)
{
    const char* filename = luaL_checkstring(L, 1);
    Data* self = new_data(L);
    size_t len;
    void* data = fs_read(filename, &len);
    if(!data)
    {
        luaL_error(L, "could not open file '%s'", filename);
    }
    self->data = data;
    self->len = len;
    return 1;
}

static int data_toString(lua_State* L)
{
    Data* self = (Data*)luaL_checkudata(L, 1, CLASS_NAME);
    lua_pushlstring(L, self->data, self->len);
    return 1;
}

static const luaL_Reg reg[] = {
    { "__gc", data_gc },
    { "fromFile", data_fromFile },
    { "toString", data_toString },
    { NULL, NULL }
};

int luaopen_data(lua_State* L)
{
    luaL_newmetatable(L, CLASS_NAME);
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}
