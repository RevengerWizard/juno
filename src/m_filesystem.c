#include <stdlib.h>

#include "luax.h"

#include "fs.h"

static void check_error(lua_State* L, int err, const char* str)
{
    if(!err)
        return;
    if(err == FS_ENOWRITEPATH || !str)
    {
        luaL_error(L, "%s", fs_errorStr(err));
    }
    luaL_error(L, "%s '%s'", fs_errorStr(err), str);
}

static int filesystem_mount(lua_State* L)
{
    const char* path = luaL_checkstring(L, 1);
    int res = fs_mount(path);
    if(res != FS_ESUCCESS)
    {
        lua_pushnil(L);
        lua_pushfstring(L, "%s '%s'", fs_errorStr(res), path);
        return 2;
    }
    lua_pushboolean(L, 1);
    return 1;
}

static int filesystem_unmount(lua_State* L)
{
    const char* path = luaL_checkstring(L, 1);
    fs_unmount(path);
    return 0;
}

static int filesystem_setWritePath(lua_State* L)
{
    const char* path = luaL_checkstring(L, 1);
    int res = fs_setWritePath(path);
    check_error(L, res, path);
    return 0;
}

static int filesystem_exists(lua_State* L)
{
    const char* filename = luaL_checkstring(L, 1);
    lua_pushboolean(L, fs_exists(filename));
    return 1;
}

static int filesystem_read(lua_State* L)
{
    const char* filename = luaL_checkstring(L, 1);
    size_t len;
    char* data = fs_read(filename, &len);
    if(!data)
    {
        luaL_error(L, "could not read file '%s'", filename);
    }
    lua_pushlstring(L, data, len);
    free(data);
    return 1;
}

static int filesystem_write(lua_State* L)
{
    const char* filename = luaL_checkstring(L, 1);
    size_t len;
    const char* data = luaL_checklstring(L, 2, &len);
    int res = fs_write(filename, data, len);
    check_error(L, res, filename);
    return 0;
}

static int filesystem_delete(lua_State* L)
{
    const char* filename = luaL_checkstring(L, 1);
    int res = fs_delete(filename);
    if(res != FS_ESUCCESS)
    {
        lua_pushnil(L);
        lua_pushfstring(L, "%s", fs_errorStr(res));
        return 2;
    }
    lua_pushboolean(L, 1);
    return 1;
}

static int filesystem_getSize(lua_State* L)
{
    const char* filename = luaL_checkstring(L, 1);
    size_t sz;
    int res = fs_size(filename, &sz);
    check_error(L, res, filename);
    lua_pushnumber(L, sz);
    return 1;
}

static int filesystem_getModified(lua_State* L)
{
    const char* filename = luaL_checkstring(L, 1);
    unsigned t;
    int res = fs_modified(filename, &t);
    check_error(L, res, filename);
    lua_pushnumber(L, t);
    return 1;
}

static int filesystem_isDir(lua_State* L)
{
    const char* filename = luaL_checkstring(L, 1);
    lua_pushboolean(L, fs_isDir(filename));
    return 1;
}

static int filesystem_listDir(lua_State* L)
{
    const char* path = luaL_checkstring(L, 1);
    fs_FileListNode* list = fs_listDir(path);
    lua_newtable(L);
    int i = 1;
    fs_FileListNode* n = list;
    while(n)
    {
        lua_pushstring(L, n->name);
        lua_rawseti(L, -2, i);
        i++;
        n = n->next;
    }
    fs_freeFileList(list);
    return 1;
}

static int filesystem_append(lua_State* L)
{
    const char* filename = luaL_checkstring(L, 1);
    size_t len;
    const char* data = luaL_checklstring(L, 2, &len);
    int res = fs_append(filename, data, len);
    check_error(L, res, filename);
    return 0;
}

static int filesystem_makeDirs(lua_State* L)
{
    const char* path = luaL_checkstring(L, 1);
    int res = fs_makeDirs(path);
    if(res != FS_ESUCCESS)
    {
        luaL_error(L, "%s '%s'", fs_errorStr(res), path);
    }
    return 0;
}

static const luaL_Reg reg[] = {
    { "mount", filesystem_mount },
    { "unmount", filesystem_unmount },
    { "setWritePath", filesystem_setWritePath },
    { "read", filesystem_read },
    { "exists", filesystem_exists },
    { "read", filesystem_read },
    { "write", filesystem_write },
    { "delete", filesystem_delete },
    { "getSize", filesystem_getSize },
    { "getModified", filesystem_getModified },
    { "isDir", filesystem_isDir },
    { "listDir", filesystem_listDir },
    { "append", filesystem_append },
    { "makeDirs", filesystem_makeDirs },
    { NULL, NULL }
};

int luaopen_filesystem(lua_State* L)
{
    luaL_newlib(L, reg);
    atexit(fs_deinit);
    return 1;
}