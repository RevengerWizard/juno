#include <stdbool.h>
#include <string.h>

#include <SDL.h>

#include "common.h"
#include "luax.h"
#include "m_source.h"

static bool inited = 0;
static double samplerate = 0;

static void audio_callback(void* udata, Uint8* stream, int size)
{
    lua_State* L = (lua_State*)udata;
    int16_t* buffer = (int16_t*)stream;
    int len = size / sizeof(int16_t);

    /* Process source commands */
    source_processCommands(L);

    /* Process sources audio */
    source_processAllSources(len);

    /* Copy master to buffer */
    Source* master = source_getMaster(NULL);
    for(int i = 0; i < len; i++)
    {
        int x = master->buf[i];
        buffer[i] = CLAMP(x, -32768, 32767);
    }
}

static int l_audio_init(lua_State* L)
{
    if(inited)
    {
        luaL_error(L, "audio is already inited");
    }

    if(SDL_Init(SDL_INIT_AUDIO) != 0)
    {
        luaL_error(L, "could not init audio");
    }

    /* Init format, open and start */
    SDL_AudioSpec fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.freq = 44100;
    fmt.format = AUDIO_S16SYS;
    fmt.channels = 2;
    fmt.callback = audio_callback;
    fmt.samples = 2048;
    fmt.userdata = L;

    if(SDL_OpenAudio(&fmt, NULL) != 0)
    {
        luaL_error(L, "could not open audio: %s", SDL_GetError());
    }

    samplerate = 44100;
    inited = true;
    source_setSamplerate(samplerate);

    /* Start audio */
    SDL_PauseAudio(0);

    return 0;
}

static const luaL_Reg reg[] = {
    { "init", l_audio_init },
    { NULL, NULL }
};

int luaopen_audio(lua_State* L)
{
    luaL_newlib(L, reg);

    /* Add .master Source field */
    int ref;
    source_getMaster(&ref);
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
    lua_setfield(L, -2, "master");
    return 1;
}