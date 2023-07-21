CC = gcc
#CDEBUG = -g -O0
CFLAGS = -std=c99 $(CDEBUG) $(INCLUDE)
RM = del

INCLUDE = -IC:/SDL2/include -IC:/LuaJIT-2.1/src -Isrc/embed -Ilib/ -Ilib/vec -Ilib/sera
LIBS = -LC:/SDL2/lib -LC:/LuaJIT-2.1/src

LDFLAGS = $(LIBS) -lmingw32 -lSDL2main -lSDL2 -llua51

SRC = $(wildcard src/*.c)
SRC_HEADERS = $(wildcard src/*.h)
SRC_OBJS = $(SRC:.c=.o)

EMBED_LUA_FILES = $(wildcard src/embed/*.lua)
EMBED_FONT_FILES = $(wildcard src/embed/*.ttf)
EMBED_C_HEADERS = $(patsubst src/embed/%.lua, src/embed/%_lua.h, $(EMBED_LUA_FILES))
EMBED_C_HEADERS += $(patsubst src/embed/%.ttf, src/embed/%_ttf.h, $(EMBED_FONT_FILES))

LIB_FILTER = lib/miniz.c
LIB_SRC = $(filter-out $(LIB_FILTER),$(wildcard lib/**/*.c))
LIB_SRC += $(filter-out $(LIB_FILTER),$(wildcard lib/*.c))
LIB_OBJS = $(LIB_SRC:.c=.o)

OBJS = $(SRC_OBJS) $(LIB_OBJS)

TARGET = bin/juno.exe

all:	$(TARGET)

embed:	$(EMBED_C_HEADERS)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

src/juno.o: src/juno.c $(EMBED_C_HEADERS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

lib/vec/%.o: lib/vec/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

lib/sera/%.o: lib/sera/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

src/embed/%_lua.h: src/embed/%.lua
	python cembed.py $< > $@

src/embed/%_ttf.h: src/embed/%.ttf
	python cembed.py $< > $@

clean:
	$(RM) $(OBJS) $(EMBED_C_HEADERS) $(TARGET)

.PHONY: embed clean