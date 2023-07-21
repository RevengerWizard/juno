juno.keyboard = juno.keyboard or {}

local keysDown = {}
local keysPressed = {}

function juno.keyboard._event(e)
    if e.type == "keypressed" then
        keysDown[e.key] = true
        keysPressed[e.key] = true
    elseif e.type == "keyreleased" then
        keysDown[e.key] = nil
    end
end

function juno.keyboard.reset()
    for k in pairs(keysPressed) do
        keysPressed[k] = nil
    end
end

function juno.keyboard.isDown(...)
    for i = 1, select("#", ...) do
        local k = select(i, ...)
        if keysDown[k] then
            return true
        end
    end
    return false
end

function juno.keyboard.wasPressed(...)
    for i = 1, select("#", ...) do
        local k = select(i, ...)
        if keysPressed[k] then
            return true
        end
    end
    return false
end