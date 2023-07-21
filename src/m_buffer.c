#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "luax.h"
#include "sera/sera.h"
#include "m_buffer.h"
#include "fs.h"

#define CLASS_NAME  BUFFER_CLASS_NAME

Buffer* buffer_new(lua_State* L)
{
    Buffer* self = (Buffer*)lua_newuserdata(L, sizeof(*self));
    luaL_setmetatable(L, CLASS_NAME);
    memset(self, 0, sizeof(*self));
    return self;
}

static int buffer_gc(lua_State* L)
{
    Buffer* self = (Buffer*)luaL_checkudata(L, 1, CLASS_NAME);
    if(self->buffer)
    {
        sr_destroyBuffer(self->buffer);
    }
    return 0;
}

static int load_buffer(Buffer* self, const void* data, int len)
{
    int w, h;
    void* pixels = stbi_load_from_memory(data, len, &w, &h, NULL, STBI_rgb_alpha);
    if(!pixels)
    {
        return -1;
    }
    self->buffer = sr_newBuffer(w, h);
    if(!self->buffer)
    {
        free(pixels);
        return -1;
    }
    sr_loadPixels(self->buffer, pixels, SR_FMT_RGBA);
    free(pixels);
    return 0;
}

static int buffer_fromFile(lua_State* L)
{
    const char* filename = luaL_checkstring(L, 1);
    Buffer* self = buffer_new(L);
    size_t len;
    void* data = fs_read(filename, &len);
    if(!data)
    {
        luaL_error(L, "could not open file '%s'", filename);
    }
    int err = load_buffer(self, data, len);
    free(data);
    if(err)
    {
        luaL_error(L, "could not load buffer");
    }
    return 1;
}

static int buffer_getWidth(lua_State* L)
{
    Buffer* self = (Buffer*)luaL_checkudata(L, 1, CLASS_NAME);
    lua_pushnumber(L, self->buffer->w);
    return 1;
}

static int buffer_getHeight(lua_State* L)
{
    Buffer* self = (Buffer*)luaL_checkudata(L, 1, CLASS_NAME);
    lua_pushnumber(L, self->buffer->h);
    return 1;
}

static int buffer_clone(lua_State* L)
{
    Buffer* self = (Buffer*)luaL_checkudata(L, 1, CLASS_NAME);
    Buffer* b = buffer_new(L);
    b->buffer = sr_cloneBuffer(self->buffer);
    if(!b->buffer)
    {
        luaL_error(L, "could not clone buffer");
    }
    return 1;
}

static int buffer_reset(lua_State* L)
{
    Buffer* self = (Buffer*)luaL_checkudata(L, 1, CLASS_NAME);
    sr_reset(self->buffer);
    return 0;
}

static const luaL_Reg reg[] = {
    { "__gc", buffer_gc },
    { "fromFile", buffer_fromFile },
    { "getWidth", buffer_getWidth },
    { "getHeight", buffer_getHeight },
    { "clone", buffer_clone },
    { "reset", buffer_reset },
    { NULL, NULL }
};

int luaopen_buffer(lua_State* L)
{
    luaL_newmetatable(L, CLASS_NAME);
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}