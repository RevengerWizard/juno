#ifndef M_BUFFER_H
#define M_BUFFER_H

#include "luax.h"
#include "sera/sera.h"

#define BUFFER_CLASS_NAME   "Buffer"

typedef struct
{
    sr_Buffer* buffer;
} Buffer;

Buffer* buffer_new(lua_State* L);

#endif