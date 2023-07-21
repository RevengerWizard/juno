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
    textinput = function(e)
        call(juno.textinput, e.text)
    end,
    resize = function(e)
        call(juno.resize, e.w, e.h)
    end,
    quit = function(e)
        call(juno.quit)
    end
}

function juno.run()
    for i, e in ipairs(juno.system.poll()) do
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

if juno.arg[2] then
    for i=2, #juno.arg do
        juno.filesystem.mount(juno.arg[i])
    end
end

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

if juno.filesystem.exists("main.lua") then
    require "main"
end

call(juno.load)