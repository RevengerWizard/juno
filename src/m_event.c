#include <stdbool.h>

#include <SDL.h>

#include "luax.h"
#include "mapping.h"

static int l_event_poll(lua_State* L)
{
    /* Create events table */
    lua_newtable(L);

    /* Poll events */
    int event = 1;
    SDL_Event e;
    while(SDL_PollEvent(&e))
    {
        lua_newtable(L);

        switch(e.type)
        {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                SDL_Scancode k = e.key.keysym.scancode;

                bool down = e.type == SDL_KEYDOWN;

                luax_setfield_string(L, "type", down ? "keypressed" : "keyreleased");
                luax_setfield_string(L, "key", keyboard_names[k]);
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
            case SDL_MOUSEBUTTONUP:
            {
                Uint8 b = e.button.button;

                bool down = e.type == SDL_MOUSEBUTTONDOWN;

                luax_setfield_string(L, "type", down ? "mousepressed" : "mousereleased");
                luax_setfield_string(L, "button", mouse_names[b]);
                luax_setfield_number(L, "x", e.button.x);
                luax_setfield_number(L, "y", e.button.y);
                break;
            }
            case SDL_MOUSEWHEEL:
            {
                luax_setfield_string(L, "type", "wheelmoved");
                luax_setfield_number(L, "x", e.wheel.x);
                luax_setfield_number(L, "y", e.wheel.y);
                break;
            }
            case SDL_TEXTINPUT:
            {
                const char* txt = e.text.text;
                luax_setfield_string(L, "type", "textinput");
                luax_setfield_string(L, "text", txt);
                break;
            }
            case SDL_TEXTEDITING:
            {
                luax_setfield_string(L, "type", "textedited");
                luax_setfield_string(L, "text", e.edit.text);
                luax_setfield_number(L, "start", e.edit.start);
                luax_setfield_number(L, "length", e.edit.length);
                break;
            }
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
            {
                bool down = e.type == SDL_JOYBUTTONDOWN;

                luax_setfield_string(L, "type", down ? "joystickpressed" : "joystickreleased");
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
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
            {
                bool down = e.type == SDL_CONTROLLERBUTTONDOWN;

                const char* button;
                if(e.cbutton.button < 0)
                {
                    button = "unknown";
                }
                else
                {
                    button = gamepad_names[e.cbutton.button];
                }

                luax_setfield_string(L, "type", down ? "gamepadpressed" : "gamepadreleased");
                luax_setfield_number(L, "joystick", e.cbutton.which);
                luax_setfield_string(L, "button", button);
                break;
            }
            case SDL_JOYDEVICEADDED:
            {
                luax_setfield_string(L, "type", "joystickadded");
                luax_setfield_number(L, "joystick", e.jdevice.which);
                break;
            }
            case SDL_JOYDEVICEREMOVED:
            {
                luax_setfield_string(L, "type", "joystickremoved");
                luax_setfield_number(L, "joystick", e.jdevice.which);
                break;
            }
            case SDL_WINDOWEVENT:
            {
                switch(e.window.event)
                {
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                    {
                        luax_setfield_string(L, "type", "focus");
                        luax_setfield_boolean(L, "focus", e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED);
                        break;
                    }
                    case SDL_WINDOWEVENT_ENTER:
                    case SDL_WINDOWEVENT_LEAVE:
                    {
                        luax_setfield_string(L, "type", "mousefocus");
                        luax_setfield_boolean(L, "focus", e.window.event == SDL_WINDOWEVENT_ENTER);
                        break;
                    }
                    case SDL_WINDOWEVENT_SHOWN:
                    case SDL_WINDOWEVENT_HIDDEN:
                    {
                        luax_setfield_string(L, "type", "visible");
                        luax_setfield_boolean(L, "visible", e.window.event == SDL_WINDOWEVENT_SHOWN);
                        break;
                    }
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
            case SDL_APP_TERMINATING:
            case SDL_QUIT:
                luax_setfield_string(L, "type", "quit");
                break;
            default:
                break;
        }

        /* Push event to events table */
        lua_rawseti(L, -2, event++);
    }
    return 1;
}

static int l_event_pump(lua_State* L)
{
    SDL_PumpEvents();
    return 0;
}

static int l_event_quit(lua_State* L)
{
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
    return 0;
}

static const luaL_Reg reg[] = {
    { "poll", l_event_poll },
    { "pump", l_event_pump },
    { "quit", l_event_quit },
    { NULL, NULL }
};

int luaopen_event(lua_State* L)
{
    luaL_newlib(L, reg);
    return 1;
}