#if _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#endif

#include "luax.h"

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

static int l_system_info(lua_State* L)
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
    { "info", l_system_info },
    { NULL, NULL }
};

int luaopen_system(lua_State* L)
{
    luaL_newlib(L, reg);
    return 1;
}