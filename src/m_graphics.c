#include <SDL.h>

#include "m_buffer.h"
#include "sera/sera.h"
#include "luax.h"

typedef struct
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
} SDLWrapper;

extern SDLWrapper* sdlwrap;

extern sr_Buffer* screen;

static const char* styles[] = { "fill", "line", NULL };

static sr_Pixel get_color(lua_State* L, int first)
{
    int r = luaL_optnumber(L, first, 255);
    int g = luaL_optnumber(L, first + 1, 255);
    int b = luaL_optnumber(L, first + 2, 255);
    int a = luaL_optnumber(L, first + 3, 255);
    return sr_pixel(r, g, b, a);
}

static sr_Rect get_rect(lua_State* L, int idx)
{
    luaL_checktype(L, idx, LUA_TTABLE);
    idx = lua_absindex(L, idx);
    lua_getfield(L, idx, "x");
    int x = lua_tonumber(L, -1);
    lua_getfield(L, idx, "y");
    int y = lua_tonumber(L, -1);
    lua_getfield(L, idx, "w");
    int w = lua_tonumber(L, -1);
    lua_getfield(L, idx, "h");
    int h = lua_tonumber(L, -1);
    lua_pop(L, 4);
    return sr_rect(x, y, w, h);
}

static void check_subrect(lua_State* L, int idx, sr_Buffer* b, sr_Rect* r)
{
    if(r->x < 0 || r->y < 0 || r->x + r->w > b->w || r->y + r->h > b->h)
    {
        luaL_argerror(L, idx, "sub rectangle out of bounds");
    }
}

static int graphics_init(lua_State* L)
{
    int screenWidth = luaL_checkint(L, 1);
    int screenHeight = luaL_checkint(L, 2);

    extern int WIDTH, HEIGHT;
    WIDTH = screenWidth;
    HEIGHT = screenHeight;

    sr_destroyBuffer(screen);
    SDL_DestroyTexture(sdlwrap->texture);

    screen = sr_newBuffer(WIDTH, HEIGHT);
    sr_setClip(screen, sr_rect(0, 0, WIDTH, HEIGHT));

    sdlwrap->texture = SDL_CreateTexture(sdlwrap->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    
    return 0;
}

static int graphics_setAlpha(lua_State* L)
{
    sr_setAlpha(screen, luaL_optnumber(L, 1, 255));
    return 0;
}

static int graphics_setBlend(lua_State* L)
{
    const char* modes[] = { "alpha", "color", "add", "subtract", "multiply", "lighten", "darken", "screen", "difference", NULL };

    int mode = luaL_checkoption(L, 1, "alpha", modes);
    sr_setBlend(screen, mode);
    return 0;
}

static int graphics_setColor(lua_State* L)
{
    sr_setColor(screen, get_color(L, 1));
    return 0;
}

static int graphics_clear(lua_State* L)
{
    sr_clear(screen, get_color(L, 1));
    return 0;
}

static int graphics_pixel(lua_State* L)
{
    int x = luaL_checknumber(L, 1);
    int y = luaL_checknumber(L, 2);
    sr_drawPixel(screen, get_color(L, 3), x, y);
    return 0;
}

static int graphics_rectangle(lua_State* L)
{
    int id = luaL_checkoption(L, 1, NULL, styles);
    int x = luaL_checknumber(L, 2);
    int y = luaL_checknumber(L, 3);
    int w = luaL_checknumber(L, 4);
    int h = luaL_checknumber(L, 5);
    sr_Pixel px = get_color(L, 6);
    if(id == 0)
        sr_drawRect(screen, px, x, y, w, h);
    else
        sr_drawBox(screen, px, x, y, w, h);
    return 0;
}

static int graphics_circle(lua_State* L)
{
    int id = luaL_checkoption(L, 1, NULL, styles);
    int x = luaL_checknumber(L, 2);
    int y = luaL_checknumber(L, 3);
    int r = luaL_checknumber(L, 4);
    sr_Pixel px = get_color(L, 5);
    if(id == 0)
        sr_drawCircle(screen, px, x, y, r);
    else
        sr_drawRing(screen, px, x, y, r);
    return 0;
}

static int graphics_line(lua_State* L)
{
    int x1 = luaL_checknumber(L, 1);
    int y1 = luaL_checknumber(L, 2);
    int x2 = luaL_checknumber(L, 3);
    int y2 = luaL_checknumber(L, 4);
    sr_Pixel px = get_color(L, 5);
    sr_drawLine(screen, px, x1, y1, x2, y2);
    return 0;
}

static int graphics_draw(lua_State* L)
{
    int hasSub = 0;
    sr_Rect sub;
    sr_Transform t;
    Buffer* src = luaL_checkudata(L, 1, BUFFER_CLASS_NAME);
    int x = luaL_optnumber(L, 2, 0);
    int y = luaL_optnumber(L, 3, 0);
    if(!lua_isnoneornil(L, 4))
    {
        hasSub = 1;
        sub = get_rect(L, 4);
        check_subrect(L, 4, src->buffer, &sub);
    }
    t.r = luaL_optnumber(L, 5, 0);
    t.sx = luaL_optnumber(L, 6, 1);
    t.sy = luaL_optnumber(L, 7, t.sx);
    t.ox = luaL_optnumber(L, 8, 0);
    t.oy = luaL_optnumber(L, 9, 0);
    sr_drawBuffer(screen, src->buffer, x, y, hasSub ? &sub : NULL, &t);
    return 0;
}

static const luaL_Reg reg[] = {
    { "init", graphics_init },
    { "setAlpha", graphics_setAlpha },
    { "setBlend", graphics_setBlend },
    { "setColor", graphics_setColor },
    { "clear", graphics_clear },
    { "pixel", graphics_pixel },
    { "line", graphics_line },
    { "rectangle", graphics_rectangle },
    { "circle", graphics_circle },
    { "draw", graphics_draw },
    { NULL, NULL }
};

int luaopen_graphics(lua_State* L)
{
    luaL_newlib(L, reg);
    return 1;
}