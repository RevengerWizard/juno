#if _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#endif

#include <SDL.h>

#include "luax.h"
#include "m_keyboard.h"

static const char* button_str(int id)
{
    switch(id)
    {
        case SDL_BUTTON_LEFT:
            return "left";
        case SDL_BUTTON_MIDDLE:
            return "middle";
        case SDL_BUTTON_RIGHT:
            return "right";
        case SDL_BUTTON_X1:
            return "wheelup";
        case SDL_BUTTON_X2:
            return "wheeldown";
        default:
            return "unknown";
    }
}

static int system_poll(lua_State* L)
{
    lua_newtable(L);

    int event = 1;
    SDL_Event e;
    while(SDL_PollEvent(&e))
    {
        lua_newtable(L);

        switch(e.type)
        {
            case SDL_QUIT:
                luax_setfield_string(L, "type", "quit");
                break;
            case SDL_KEYDOWN:
            {
                SDL_Scancode k = e.key.keysym.scancode;
                const char* name = scancode_names[k] ? scancode_names[k] : "unknown";

                luax_setfield_string(L, "type", "keypressed");
                luax_setfield_string(L, "key", name);
                break;
            }
            case SDL_KEYUP:
            {
                SDL_Scancode k = e.key.keysym.scancode;
                const char* name = scancode_names[k] ? scancode_names[k] : "unknown";

                luax_setfield_string(L, "type", "keyreleased");
                luax_setfield_string(L, "key", name);
                break;
            }
            case SDL_MOUSEMOTION:
            {
                extern int mouse_x, mouse_y;
                luax_setfield_string(L, "type", "mousemoved");
                luax_setfield_number(L, "x", mouse_x);
                luax_setfield_number(L, "y", mouse_y);
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
                luax_setfield_string(L, "type", "mousepressed");
                luax_setfield_string(L, "button", button_str(e.button.button));
                luax_setfield_number(L, "x", e.button.x);
                luax_setfield_number(L, "y", e.button.y);
                break;
            case SDL_MOUSEBUTTONUP:
                luax_setfield_string(L, "type", "mousereleased");
                luax_setfield_string(L, "button", button_str(e.button.button));
                luax_setfield_number(L, "x", e.button.x);
                luax_setfield_number(L, "y", e.button.y);
                break;
            case SDL_TEXTINPUT:
            {
                const char* txt = e.text.text;
                luax_setfield_string(L, "type", "textinput");
                luax_setfield_string(L, "text", txt);
                break;
            }
            case SDL_WINDOWEVENT:
            {
                switch(e.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    {
                        int width = e.window.data1;
                        int height = e.window.data2;
                        luax_setfield_string(L, "type", "resize");
                        luax_setfield_number(L, "w", width);
                        luax_setfield_number(L, "h", height);
                        break;
                    }
                }
                break;
            }
        }

        lua_rawseti(L, -2, event++);
    }
    return 1;
}

static char* dirname(char* str)
{
    char* p = str + strlen(str);
    while(p != str)
    {
        if(*p == '/' || *p == '\\')
        {
            *p = '\0';
            break;
        }
        p--;
    }
    return str;
}

static const char* os_name(void)
{
#if _WIN32
    return "windows";
#elif __linux__
    return  "linux";
#elif __FreeBSD__
    return "bsd";
#elif __APPLE__
    return "osx";
#else
    return "?";
#endif    
}

static int system_info(lua_State* L)
{
    const char* opts[] = { "os", "exedir", "appdata", NULL };
    
    int id = luaL_checkoption(L, 1, NULL, opts);

    switch(id)
    {
        case 0:
            lua_pushstring(L, os_name());
            break;
        case 1:
#if _WIN32
            char buf[1024];
            int len = GetModuleFileName(NULL, buf, sizeof(buf) - 1);
            buf[len] = '\0';
            dirname(buf);
            lua_pushfstring(L, "%s", buf);
#elif __linux__
            char path[128];
            char buf[1024];
            sprintf(path, "/proc/%d/exe", getpid());
            int len = readlink(path, buf, sizeof(buf) - 1);
            buf[len] = '\0';
            dirname(buf);
            lua_pushfstring(L, "%s", buf);
#elif __FreeBSD__
            /* TODO : Implement this */
            lua_pushfstring(L, ".");
#elif __APPLE__
            char buf[1024];
            uint32_t size = sizeof(buf);
            dirname(buf);
            lua_pushfstring(L, "%s", buf);
#else
            lua_pushfstring(L, ".");
#endif
            break;
        case 2:
#if _WIN32
            lua_pushfstring(L, "%s", getenv("APPDATA"));
#elif __APPLE__
            lua_pushfstring(L, "%s/Library/Application Support", getenv("HOME"));
#else
            lua_pushfstring(L, "%s/.local/share", getenv("HOME"));
#endif
            break;
    }

    return 1;
}

static const luaL_Reg reg[] = {
    { "poll", system_poll },
    { "info", system_info },
    { NULL, NULL }
};

int luaopen_system(lua_State* L)
{
    luaL_newlib(L, reg);
    return 1;
}