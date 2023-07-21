#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "luax.h"
#include "sera/sera.h"
#include "fs.h"
#include "ttf.h"
#include "m_buffer.h"

#define CLASS_NAME  "Font"
#define DEFAULT_FONTSIZE 8

typedef struct
{
    ttf_Font* font;
} Font;

static Font* font_new(lua_State* L)
{
    Font* self = (Font*)lua_newuserdata(L, sizeof(*self));
    luaL_setmetatable(L, CLASS_NAME);
    memset(self, 0, sizeof(*self));
    return self;
}

static int font_gc(lua_State* L)
{
    Font* self = (Font*)luaL_checkudata(L, 1, CLASS_NAME);
    if(self->font)
    {
        ttf_destroy(self->font);
    }
    return 0;
}

static const char* load_font(Font* self, const void* data, int len, int ptsize)
{
    self->font = ttf_new(data, len);
    if(!self->font)
    {
        return "could not load font";
    }
    ttf_ptsize(self->font, ptsize);
    return NULL;
}

static int font_fromFile(lua_State* L)
{
    const char* filename = luaL_checkstring(L, 1);
    int fontsize = luaL_optint(L, 2, DEFAULT_FONTSIZE);
    Font* self = font_new(L);
    size_t len;
    void* data = fs_read(filename, &len);
    /* Load new font */
    if(!data)
    {
        luaL_error(L, "could not open file '%s'", filename);
    }
    const char* err = load_font(self, data, len, fontsize);
    free(data);
    if(err)
        luaL_error(L, "%s", err);
    return 1;
}

static int font_fromEmbedded(lua_State* L)
{
#include "font_ttf.h"

    int fontsize = luaL_optint(L, 1, DEFAULT_FONTSIZE);
    Font* self = font_new(L);
    const char* err = load_font(self, font_ttf, sizeof(font_ttf), fontsize);
    if(err)
        luaL_error(L, "%s", err);
    return 1;
}

static int font_render(lua_State* L)
{
    int w, h;
    Font* self = (Font*)luaL_checkudata(L, 1, CLASS_NAME);
    const char* str = lua_tostring(L, 2);
    if(!str || *str == '\0')
        str = " ";
    Buffer* b = buffer_new(L);
    void* data = ttf_render(self->font, str, &w, &h);
    if(!data)
    {
        luaL_error(L, "could not render text");
    }
    /* Load bitmap and free intermediate 8bit bitmap */
    b->buffer = sr_newBuffer(w, h);
    if(!b->buffer)
    {
        free(data);
        luaL_error(L, "could not create buffer");
    }
    sr_loadPixels8(b->buffer, data, NULL);
    free(data);
    return 1;
}

static int font_getWidth(lua_State* L)
{
    Font* self = (Font*)luaL_checkudata(L, 1, CLASS_NAME);
    const char* str = luaL_checkstring(L, 2);
    lua_pushnumber(L, ttf_width(self->font, str));
    return 1;
}

static int font_getHeight(lua_State* L)
{
    Font* self = (Font*)luaL_checkudata(L, 1, CLASS_NAME);
    lua_pushnumber(L, ttf_height(self->font));
    return 1;
}

static const luaL_Reg reg[] = {
    { "__gc", font_gc },
    { "fromFile", font_fromFile },
    { "fromEmbedded", font_fromEmbedded },
    { "render", font_render },
    { "getWidth", font_getWidth },
    { "getHeight", font_getHeight },
    { NULL, NULL }
};

int luaopen_font(lua_State* L)
{
    luaL_newmetatable(L, CLASS_NAME);
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}