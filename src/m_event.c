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

static int event_poll(lua_State* L)
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
            case SDL_JOYBUTTONDOWN:
            {
                luax_setfield_string(L, "type", "joystickpressed");
                luax_setfield_number(L, "joystick", e.jbutton.which);
                luax_setfield_number(L, "button", e.jbutton.button);
                break;
            }
            case SDL_JOYAXISMOTION:
            {
                luax_setfield_string(L, "type", "joystickaxis");
                luax_setfield_number(L, "joystick", e.jaxis.which);
                luax_setfield_number(L, "axis", e.jaxis.axis);
                int value = e.jaxis.value;
                luax_setfield_number(L, "value", ((double)value + 0.5f) / (INT16_MAX + 0.5f));
                break;
            }
            case SDL_JOYBUTTONUP:
            {
                luax_setfield_string(L, "type", "joystickreleased");
                luax_setfield_number(L, "joystick", e.jbutton.which);
                luax_setfield_number(L, "button", e.jbutton.button);
                break;
            }
            case SDL_JOYHATMOTION:
            {
                luax_setfield_string(L, "type", "joystickhat");
                luax_setfield_number(L, "joystick", e.jhat.which);
                luax_setfield_number(L, "hat", e.jhat.hat);

                lua_newtable(L); /* e.state */
                luax_setfield_boolean(L, "up", e.jhat.value & SDL_HAT_UP);
                luax_setfield_boolean(L, "down", e.jhat.value & SDL_HAT_DOWN);
                luax_setfield_boolean(L, "left", e.jhat.value & SDL_HAT_LEFT);
                luax_setfield_boolean(L, "right", e.jhat.value & SDL_HAT_RIGHT);
                lua_setfield(L, -2, "state"); /* push state to event table */
                break;
            }
            case SDL_JOYBALLMOTION:
            {
                luax_setfield_string(L, "type", "joystickball");
                luax_setfield_number(L, "joystick", e.jball.which);
                luax_setfield_number(L, "ball", e.jball.ball);
                luax_setfield_number(L, "x", e.jball.xrel);
                luax_setfield_number(L, "y", e.jball.yrel);
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

static const luaL_Reg reg[] = {
    { "poll", event_poll },
    { NULL, NULL }
};

int luaopen_event(lua_State* L)
{
    luaL_newlib(L, reg);
    return 1;
}