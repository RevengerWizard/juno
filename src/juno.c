#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "luax.h"
#include <SDL.h>

#include "luax.h"
#include "sera/sera.h"
#include "m_juno.h"

#define MAX_FPS 30.0

#define SDL_WRAPPER "sdl2.wrapper"

typedef struct
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
} SDLWrapper;

SDLWrapper* sdlwrap = NULL;
sr_Buffer* screen = NULL;
SDL_Rect dst;

int WIDTH = 200;
int HEIGHT = 200;
int mouse_x = 0;
int mouse_y = 0;

static int wrapper_gc(lua_State* L)
{
    SDLWrapper* wrapper = (SDLWrapper*)lua_touserdata(L, 1);
    SDL_DestroyTexture(wrapper->texture);
    SDL_DestroyRenderer(wrapper->renderer);
    SDL_DestroyWindow(wrapper->window);
	wrapper->window = NULL;
	wrapper->renderer = NULL;
    wrapper->texture = NULL;
    return 0;
}

static double screen_scale(int ww, int wh) 
{
    int sw = WIDTH;
    int sh = HEIGHT;

    int w = ww + sw;
    int h = wh + sh;

    /* Pixel perfect */
    w -= w % sw;
    h -= h % sh;

    while(w > ww)
        w -= sw;
    while(h > wh)
        h -= sh;

    return (double)w / (double)sw < (double)h / (double)sh
        ? (double)w / (double)sw
        : (double)h / (double)sh;
}

int main(int argc, char** argv)
{
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0") == SDL_FALSE)
    {
        fprintf(stderr, "Failed to set hint: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Window* window = SDL_CreateWindow("juno", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    if(window == NULL)
    {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == NULL)
    {
        fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    if(texture == NULL)
    {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    screen = sr_newBuffer(WIDTH, HEIGHT);

    /* Init lua state */
    lua_State* L = lua_open();
    luaL_openlibs(L);

    luaL_newmetatable(L, SDL_WRAPPER);
    lua_pushcfunction(L, wrapper_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    sdlwrap = (SDLWrapper*)lua_newuserdata(L, sizeof(*sdlwrap));
    sdlwrap->window = window;
    sdlwrap->renderer = renderer;
    sdlwrap->texture = texture;

    luaL_getmetatable(L, SDL_WRAPPER);
    lua_setmetatable(L, -2);
    lua_setfield(L, LUA_REGISTRYINDEX, SDL_WRAPPER);

    /* Init main module -- this also inits the submodules */
    luaL_requiref(L, "juno", luaopen_juno, 1);

    /* Push command line arguments */
    lua_getglobal(L, "juno");
    lua_newtable(L);
    for(int i = 0; i < argc; i++)
    {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i + 1);
    }
    lua_setfield(L, -2, "arg");
    lua_pop(L, 1);
    
    /* 
    * Init embedded scripts 
    * -- these should be ordered in the array in the order we want them loaded;
    * init.lua should always be last since it depends on all the other modules 
    */
#include "graphics_lua.h"
#include "keyboard_lua.h"
#include "mouse_lua.h"
#include "timer_lua.h"
#include "init_lua.h"
    struct
    {
        const char* name;
        const char* data;
        int size;
    } items[] = {
        { "graphics.lua", graphics_lua, sizeof(graphics_lua) },
        { "keyboard.lua", keyboard_lua, sizeof(keyboard_lua) },
        { "mouse.lua", mouse_lua, sizeof(mouse_lua) },
        { "timer.lua", timer_lua, sizeof(timer_lua) },
        { "init.lua", init_lua, sizeof(init_lua) },
        { NULL, NULL, 0 }
    };

    int i;
    for(i = 0; items[i].name; i++)
    {
        int err = luaL_loadbuffer(L, items[i].data, items[i].size, items[i].name);
        if(err || lua_pcall(L, 0, 0, 0) != 0)
        {
            const char* str = lua_tostring(L, -1);
            fprintf(stderr, "error: %s\n", str);
            abort();
        }
    }

    /* Do main loop */
    double last = 0;
    while(true)
    {
        lua_getglobal(L, "juno");
        if(!lua_isnil(L, -1))
        {
            lua_getfield(L, -1, "run");
            if(!lua_isnil(L, -1))
            {
                int err = lua_pcall(L, 0, 1, 0);
                if(err)
                {
                    const char* str = lua_tostring(L, -1);
                    fprintf(stderr, "error: %s\n", str);
                    break;
                }
                if(lua_isnumber(L, -1) && lua_tonumber(L, -1) == 1)
                    break;
            }
            lua_pop(L, 1);
        }

        char* l_pixels;
        int pitch;
        SDL_LockTexture(sdlwrap->texture, NULL, (void**)&l_pixels, &pitch);
        memcpy(l_pixels, screen->pixels, pitch * HEIGHT);

        SDL_UnlockTexture(texture);

        int ww, wh;
        SDL_GetWindowSize(sdlwrap->window, &ww, &wh);
        double scale = screen_scale(ww, wh);

        int w = (int)(WIDTH * scale);
        int h = (int)(HEIGHT * scale);
        int x = (ww - w) / 2;
        int y = (wh - h) / 2;
        dst = (SDL_Rect){ x, y, w, h };

        int mx, my;
        SDL_GetMouseState(&mx, &my);
        mouse_x = (mx - x) / scale;
        mouse_y = (my - y) / scale;

        SDL_SetRenderDrawColor(sdlwrap->renderer, 0, 0, 0, 255);
        SDL_RenderClear(sdlwrap->renderer);

        SDL_RenderCopy(sdlwrap->renderer, sdlwrap->texture, NULL, &dst);
        SDL_RenderPresent(sdlwrap->renderer);

        /* Wait for next frame */
        double step = 1.0 / MAX_FPS;
        double now = SDL_GetTicks64() / 1000.0;
        double wait = step - (now - last);
        last += step;
        if(wait > 0)
        {
            SDL_Delay(wait * 1000.0);
        }
        else
        {
            last = now;
        }
    }

    sr_destroyBuffer(screen);

    lua_close(L);

    SDL_CloseAudio();
    SDL_Quit();

    return EXIT_SUCCESS;
}