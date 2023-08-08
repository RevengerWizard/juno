local function call(fn, ...)
    if fn then return fn(...) end
end

local function merge(...)
    local res = {}
    for i = 1, select("#", ...) do
        local t = select(i, ...)
        if t then
            for k, v in pairs(t) do
                res[k] = v
            end
        end
    end
    return res
end

local handlers = {
    keypressed = function(e)
        call(juno.keyboard._event, e)
        call(juno.keypressed, e.key, e.char)
    end,
    keyreleased = function(e)
        call(juno.keyboard._event, e)
        call(juno.keyreleased, e.key)
    end,
    mousemoved = function(e)
        call(juno.mouse._event, e)
        call(juno.mousemoved, e.x, e.y)
    end,
    mousepressed = function(e)
        call(juno.mouse._event, e)
        call(juno.mousepressed, e.x, e.y, e.button)
    end,
    mousereleased = function(e)
        call(juno.mouse._event, e)
        call(juno.mousereleased, e.x, e.y, e.button)
    end,
    wheelmoved = function(e)
        call(juno.wheelmoved, e.x, e.y)
    end,
    joystickpressed = function(e)
        call(juno.joystickpressed, juno.joystick.joysticks[e.joystick+1], e.button)
    end,
    joystickreleased = function(e)
        call(juno.joystickreleased, juno.joystick.joysticks[e.joystick+1], e.button)
    end,
    joystickaxis = function(e)
        call(juno.joystickaxis, juno.joystick.joysticks[e.joystick+1], e.axis, e.value)
    end,
    joystickhat = function(e)
        call(juno.joystickhat, juno.joystick.joysticks[e.joystick+1], e.hat, e.state)
    end,
    joystickball = function(e)
        call(juno.joystickball, juno.joystick.joysticks[e.joystick+1], e.ball, e.x, e.y)
    end,
    gamepadpressed = function(e)
        call(juno.gamepadpressed, juno.joystick.joysticks[e.joystick+1], e.button)
    end,
    gamepadreleased = function(e)
        call(juno.gamepadreleased, juno.joystick.joysticks[e.joystick+1], e.button)
    end,
    joystickadded = function(e)
        call(juno.joystickadded, e.joystick)
    end,
    joystickremoved = function(e)
        call(juno.joystickremoved, e.joystick)
    end,
    textinput = function(e)
        call(juno.textinput, e.text)
    end,
    textedited = function(e)
        call(juno.textedited, e.text, e.start, e.length)
    end,
    focus = function(e)
        call(juno.focus, e.focus)
    end,
    mousefocus = function(e)
        call(juno.mousefocus, e.focus)
    end,
    visible = function(e)
        call(juno.visible, e.visible)
    end,
    resize = function(e)
        call(juno.resize, e.w, e.h)
    end,
    quit = function(e)
        call(juno.quit)
    end
}

function juno.run()
    juno.event.pump()
    for i, e in ipairs(juno.event.poll()) do
        if e.type == "quit" then
            call(handlers[e.type], e)
            return 0
        end
        call(handlers[e.type], e)
    end
    call(juno.timer.step)
    call(juno.update, call(juno.timer.getDelta))
    call(juno.graphics.clear, 0, 0, 0)
    call(juno.draw)
    call(juno.keyboard.reset)
    call(juno.mouse.reset)
end

-- Mount project paths
if juno.arg[2] then
    -- Try to mount all arguments as package
    for i=2, #juno.arg do
        juno.filesystem.mount(juno.arg[i])
    end
end

-- Add filesystem-compatible package loader
table.insert(package.loaders, 1, function(modname)
    modname = modname:gsub("%.", "/")
    for x in package.path:gmatch("[^;]+") do
        local filename = x:gsub("?", modname)
        if juno.filesystem.exists(filename) then
            return assert(loadstring(juno.filesystem.read(filename), "=" .. filename))
        end
    end
end)

-- Add extra package paths
package.path = package.path .. ";?/init.lua"

local c = {}
if juno.filesystem.exists("conf.lua") then
    c = call(require, "conf")
end

local conf = merge({
    title       = "untitled",
    width       = 200,
    height      = 200,
}, c)

juno.window.setTitle(conf.title)
juno.graphics.init(conf.width, conf.height)
juno.audio.init()
juno.joystick.init()

-- Open all of our joysticks and store them
juno.joystick.joysticks = {}
for i=0, juno.joystick.getCount()-1 do
    table.insert(juno.joystick.joysticks, juno.joystick.open(i))
end

if juno.filesystem.exists("main.lua") then
    require "main"
end

call(juno.load)