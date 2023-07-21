#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "common.h"
#include "luax.h"
#include "fs.h"
#include "vec/vec.h"
#include "m_source.h"

#define CLASS_NAME  SOURCE_CLASS_NAME

#define FX_BITS (12)
#define FX_UNIT (1 << FX_BITS)
#define FX_MASK (FX_UNIT - 1)
#define FX_LERP(a, b, p) ((a) + ((((b) - (a)) * (p)) >> (FX_BITS)))

static int samplerate = 44100;
static Source* master;
static int masterRef = LUA_NOREF;
static vec_t(Source*) sources;

typedef struct
{
    int type;
    Source* source;
    int i;
    double f;
    void* p;
} Command;

enum
{
    COMMAND_NULL,
    COMMAND_ADD,
    COMMAND_DESTROY,
    COMMAND_PLAY,
    COMMAND_PAUSE,
    COMMAND_STOP,
    COMMAND_SET_DESTINATION,
    COMMAND_SET_GAIN,
    COMMAND_SET_PAN,
    COMMAND_SET_RATE,
    COMMAND_SET_LOOP
};

static vec_t(Command) commands;

static Command command(int type, Source* source)
{
    Command c;
    memset(&c, 0, sizeof(c));
    c.type = type;
    c.source = source;
    return c;
}

static void push_command(Command* c)
{
    vec_push(&commands, *c);
}

static SourceEvent event(int type)
{
    SourceEvent e;
    memset(&e, 0, sizeof(e));
    e.type = type;
    return e;
}

static void emit_event(Source* s, SourceEvent* e)
{
    if(s->onEvent)
    {
        s->onEvent(s, e);
    }
}

static void recalc_gains(Source* self)
{
    double left, right;
    double pan = CLAMP(self->pan, -1, 1);
    double gain = MAX(self->gain, 0);
    /* Get linear gains */
    left = ((pan < 0) ? 1 : (1 - pan)) * gain;
    right = ((pan > 0) ? 1 : (1 + pan)) * gain;
    /* Apply curve */
    left = left * left;
    right = right * right;
    /* Set fixedpoint gains */
    self->lgain = left * FX_UNIT;
    self->rgain = right * FX_UNIT;
}

static Source* check_source(lua_State* L, int idx)
{
    Source** p = (Source**)luaL_checkudata(L, idx, CLASS_NAME);
    return *p;
}

static Source* new_source(lua_State* L)
{
    Source* self = (Source*)malloc(sizeof(*self));
    memset(self, 0, sizeof(*self));
    self->dataRef = LUA_NOREF;
    self->destRef = LUA_NOREF;
    self->gain = 1.0;
    self->pan = 0;
    recalc_gains(self);

    /* Init lua pointer to the actual Source struct */
    Source** p = (Source**)lua_newuserdata(L, sizeof(self));
    luaL_setmetatable(L, CLASS_NAME);
    *p = self;
    return self;
}

static void destroy_source(Source* self)
{
    /* Note: All the lua references of the source should be unreferenced before
     * the source is destroyed. The source should also be removed from the
     * `sources` vector -- this is done in `source_processCommands()`, the only
     * place this function should ever be called. */
    SourceEvent e = event(SOURCE_EVENT_DEINIT);
    emit_event(self, &e);
    free(self);
}

static double get_baserate(Source* self)
{
    return (double)self->samplerate / (double)samplerate;
}

static void rewind_stream(Source* self, long long position)
{
    /* Rewind stream */
    SourceEvent e = event(SOURCE_EVENT_REWIND);
    emit_event(self, &e);

    /* Process first chunk and reset */
    e = event(SOURCE_EVENT_PROCESS);
    e.offset = 0;
    e.len = SOURCE_BUFFER_MAX / 2;
    emit_event(self, &e);
    self->end = self->length;
    self->bufEnd = e.len;
    self->position = position;
}

static void onevent_wav(Source* s, SourceEvent* e)
{
    switch(e->type)
    {
        case SOURCE_EVENT_INIT:
        {
            int err = wav_read(&s->wav, s->data->data, s->data->len);
            if(err != WAV_ESUCCESS)
            {
                luaL_error(e->luaState, "could not init wav stream: %s", wav_strerror(err));
            }
            if(s->wav.bitdepth != 16)
            {
                luaL_error(e->luaState, "could not init wav stream, expected 16bit wave");
            }
            if(s->wav.channels != 1 && s->wav.channels != 2)
            {
                luaL_error(e->luaState, "could not init wav stream, expected mono/stereo wave");
            }
            s->length = s->wav.length;
            s->samplerate = s->wav.samplerate;
            break;
        }
        case SOURCE_EVENT_REWIND:
            s->wavIdx = 0;
            break;
        case SOURCE_EVENT_PROCESS:
        {
            int i, x;
            for(i = 0; i < e->len; i++)
            {
                /* Hit the end? Rewind and continue */
                if(s->wavIdx >= s->length)
                {
                    s->wavIdx = 0;
                }
                /* Process */
                int idx = (e->offset + i) & SOURCE_BUFFER_MASK;
                if(s->wav.channels == 2)
                {
                    /* Process stereo */
                    x = s->wavIdx << 1;
                    s->rawBufLeft[idx] = ((short *)s->wav.data)[x];
                    s->rawBufRight[idx] = ((short *)s->wav.data)[x + 1];
                }
                else
                {
                    /* Process mono */
                    s->rawBufLeft[idx] = s->rawBufRight[idx] = ((short *)s->wav.data)[s->wavIdx];
                }
                s->wavIdx++;
            }
            break;
        }
    }
}

static void onevent_ogg(Source* s, SourceEvent* e)
{
    switch (e->type)
    {
        case SOURCE_EVENT_INIT:
        {
            int err;
            s->oggStream = stb_vorbis_open_memory(s->data->data, s->data->len, &err, NULL);
            if(!s->oggStream)
            {
                luaL_error(e->luaState, "could not init ogg stream; bad data?");
            }
            stb_vorbis_info info = stb_vorbis_get_info(s->oggStream);
            s->samplerate = info.sample_rate;
            s->length = stb_vorbis_stream_length_in_samples(s->oggStream);
            break;
        }
        case SOURCE_EVENT_DEINIT:
            if(s->oggStream)
            {
                stb_vorbis_close(s->oggStream);
            }
            break;
        case SOURCE_EVENT_REWIND:
            stb_vorbis_seek_start(s->oggStream);
            break;
        case SOURCE_EVENT_PROCESS:
        {
            int i, n;
            short buf[SOURCE_BUFFER_MAX];
            int len = e->len * 2;
            int z = e->offset;
        fill:
            n = stb_vorbis_get_samples_short_interleaved(s->oggStream, 2, buf, len);
            n *= 2;
            for(i = 0; i < n; i += 2)
            {
                int idx = z++ & SOURCE_BUFFER_MASK;
                s->rawBufLeft[idx] = buf[i];
                s->rawBufRight[idx] = buf[i + 1];
            }
            /* Reached end of stream before the end of the buffer? rewind and fill
            * remaining buffer */
            if(len != n)
            {
                stb_vorbis_seek_start(s->oggStream);
                len -= n;
                goto fill;
            }
            break;
        }
    }
}

Source* source_getMaster(int* ref)
{
    if(ref)
    {
        *ref = masterRef;
    }
    return master;
}

void source_setSamplerate(int sr)
{
    samplerate = sr;
}

void source_processCommands(lua_State* L)
{
    int i;
    Command* c;
    vec_t(int) oldRefs;
    vec_init(&oldRefs);

    /* Handle commands */
    vec_foreach_ptr(&commands, c, i)
    {
        switch(c->type)
        {
            case COMMAND_ADD:
                vec_push(&sources, c->source);
                break;
            case COMMAND_DESTROY:
                vec_push(&oldRefs, c->source->dataRef);
                vec_push(&oldRefs, c->source->destRef);
                vec_remove(&sources, c->source);
                destroy_source(c->source);
                break;
            case COMMAND_PLAY:
                if(c->i || c->source->state == SOURCE_STATE_STOPPED)
                {
                    rewind_stream(c->source, 0);
                }
                c->source->state = SOURCE_STATE_PLAYING;
                break;
            case COMMAND_PAUSE:
                if(c->source->state == SOURCE_STATE_PLAYING)
                {
                    c->source->state = SOURCE_STATE_PAUSED;
                }
                else if(c->source->state == SOURCE_STATE_PAUSED)
                {
                    c->source->state = SOURCE_STATE_PLAYING;
                }
                break;
            case COMMAND_STOP:
                c->source->state = SOURCE_STATE_STOPPED;
                break;
            case COMMAND_SET_DESTINATION:
                vec_push(&oldRefs, c->source->destRef);
                c->source->destRef = c->i;
                c->source->dest = c->p;
                break;
            case COMMAND_SET_GAIN:
                c->source->gain = c->f;
                recalc_gains(c->source);
                break;
            case COMMAND_SET_PAN:
                c->source->pan = c->f;
                recalc_gains(c->source);
                break;
            case COMMAND_SET_RATE:
                c->source->rate = get_baserate(c->source) * c->f * FX_UNIT;
                break;
            case COMMAND_SET_LOOP:
                if(c->i)
                {
                    c->source->flags |= SOURCE_FLOOP;
                }
                else
                {
                    c->source->flags &= ~SOURCE_FLOOP;
                }
                break;
        }
    }

    /* Clear command vector */
    vec_clear(&commands);

    /* Remove old Lua references */
    if(oldRefs.length > 0)
    {
        int i, ref;
        vec_foreach(&oldRefs, ref, i)
        {
            luaL_unref(L, LUA_REGISTRYINDEX, ref);
        }
    }
    vec_deinit(&oldRefs);
}

static void source_process(Source* self, int len)
{
    int i;
    /* Replace flag still set? Zeroset the buffer */
    if(self->flags & SOURCE_FREPLACE)
    {
        memset(self->buf, 0, sizeof(*self->buf) * len);
    }
    /* Process audio stream and add to our buffer */
    if(self->state == SOURCE_STATE_PLAYING && self->onEvent)
    {
        for(i = 0; i < len; i += 2)
        {
            int idx = (self->position >> FX_BITS);
            /* Process the stream and fill the raw buffer if the next index requires
             * samples we don't yet have */
            if(idx + 1 >= self->bufEnd)
            {
                SourceEvent e = event(SOURCE_EVENT_PROCESS);
                e.offset = (self->bufEnd) & SOURCE_BUFFER_MASK;
                e.len = SOURCE_BUFFER_MAX / 2;
                emit_event(self, &e);
                self->bufEnd += e.len;
            }
            /* Have we reached the end? */
            if(idx >= self->end)
            {
                /* Not set to loop? Stop and stop processing */
                if(~self->flags & SOURCE_FLOOP)
                {
                    self->state = SOURCE_STATE_STOPPED;
                    break;
                }
                /* Set to loop: Streams always fill the raw buffer in a loop, so we
                 * just increment the end index by the stream's length so that it
                 * continues for another iteration of the sound file */
                self->end = idx + self->length;
            }
            /* Write interpolated frame to buffer */
            int p = self->position & FX_MASK;
            /* Left */
            int la = self->rawBufLeft[idx & SOURCE_BUFFER_MASK];
            int lb = self->rawBufLeft[(idx + 1) & SOURCE_BUFFER_MASK];
            self->buf[i] += FX_LERP(la, lb, p);
            /* Right */
            int ra = self->rawBufRight[idx & SOURCE_BUFFER_MASK];
            int rb = self->rawBufRight[(idx + 1) & SOURCE_BUFFER_MASK];
            self->buf[i + 1] += FX_LERP(ra, rb, p);
            /* Increment position */
            self->position += self->rate;
        }
    }

    /* Apply gains */
    for(i = 0; i < len; i += 2)
    {
        self->buf[i] = (self->buf[i] * self->lgain) >> FX_BITS;
        self->buf[i + 1] = (self->buf[i + 1] * self->rgain) >> FX_BITS;
    }
    /* Write to destination */
    if(self->dest)
    {
        if(self->dest->flags & SOURCE_FREPLACE)
        {
            memcpy(self->dest->buf, self->buf, sizeof(*self->buf) * len);
            self->dest->flags &= ~SOURCE_FREPLACE;
        }
        else
        {
            for(i = 0; i < len; i++)
            {
                self->dest->buf[i] += self->buf[i];
            }
        }
    }
    /* Reset our flag as to replace the buffer's content */
    self->flags |= SOURCE_FREPLACE;
}

void source_processAllSources(int len)
{
    int i;
    Source* s;
    /* Sources are processed in reverse (newer Sources are processed first) --
     * this assures the master is processed last */
    vec_foreach_rev(&sources, s, i)
    {
        source_process(s, len);
    }
}

static int source_gc(lua_State* L)
{
    Source* self = check_source(L, 1);
    Command c = command(COMMAND_DESTROY, self);
    push_command(&c);
    return 0;
}

static int source_fromData(lua_State* L)
{
    Data* data = (Data*)luaL_checkudata(L, 1, DATA_CLASS_NAME);
    Source* self = new_source(L);
    /* Init data reference */
    self->data = data;
    lua_pushvalue(L, 1);
    self->dataRef = luaL_ref(L, LUA_REGISTRYINDEX);

    /* Detect format and set appropriate event handler */
    /* Is .wav? */
    if(data->len > 12 && !memcmp(((char *)data->data) + 8, "WAVE", 4))
    {
        self->onEvent = onevent_wav;
        goto init;
    }
    /* Is .ogg? */
    if(data->len > 4 && !memcmp(data->data, "OggS", 4))
    {
        self->onEvent = onevent_ogg;
        goto init;
    }
    /* Made it here? Error out because we couldn't detect the format */
    luaL_error(L, "could not init Source; bad Data format?");
    /* Init stream */
init:;
    SourceEvent e = event(SOURCE_EVENT_INIT);
    e.luaState = L;
    emit_event(self, &e);
    /* Init */
    self->rate = get_baserate(self) * FX_UNIT;
    self->dest = master;
    /* Issue "add" command to push to `sources` vector */
    Command c = command(COMMAND_ADD, self);
    push_command(&c);
    return 1;
}

static int source_setLoop(lua_State* L)
{
    Source* self = check_source(L, 1);
    int loop = luax_optboolean(L, 2, 0);
    Command c = command(COMMAND_SET_LOOP, self);
    c.i = loop;
    push_command(&c);
    return 0;
}

static int source_setGain(lua_State* L)
{
    Source* self = check_source(L, 1);
    double gain = luaL_optnumber(L, 2, 1.);
    Command c = command(COMMAND_SET_GAIN, self);
    c.f = gain;
    push_command(&c);
    return 0;
}

static int source_getState(lua_State* L)
{
    Source* self = check_source(L, 1);
    switch(self->state)
    {
        case SOURCE_STATE_PLAYING:
            lua_pushstring(L, "playing");
            break;
        case SOURCE_STATE_PAUSED:
            lua_pushstring(L, "paused");
            break;
        case SOURCE_STATE_STOPPED:
            lua_pushstring(L, "stopped");
            break;
        default:
            lua_pushstring(L, "?");
            break;
    }
    return 1;
}

static int source_play(lua_State* L)
{
    Source* self = check_source(L, 1);
    int reset = luax_optboolean(L, 2, 0);
    Command c = command(COMMAND_PLAY, self);
    c.i = reset;
    push_command(&c);
    return 0;
}

static int source_pause(lua_State* L)
{
    Source* self = check_source(L, 1);
    Command c = command(COMMAND_PAUSE, self);
    push_command(&c);
    return 0;
}

static int source_stop(lua_State* L)
{
    Source* self = check_source(L, 1);
    Command c = command(COMMAND_STOP, self);
    push_command(&c);
    return 0;
}

static const luaL_Reg reg[] = {
    { "__gc", source_gc },
    { "fromData", source_fromData },
    { "getState", source_getState },
    { "setLoop", source_setLoop },
    { "setGain", source_setGain },
    { "play", source_play },
    { "pause", source_pause },
    { "stop", source_stop },
    { NULL, NULL }
};

int luaopen_source(lua_State* L)
{
    luaL_newmetatable(L, CLASS_NAME);
    luaL_setfuncs(L, reg, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    master = new_source(L);
    masterRef = luaL_ref(L, LUA_REGISTRYINDEX);
    Command c = command(COMMAND_ADD, master);
    push_command(&c);
    return 1;
}