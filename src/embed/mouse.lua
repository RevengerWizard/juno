local buttonsDown = {}
local buttonsPressed = {}

function juno.mouse._event(e)
    if e.type == "mousepressed" then
        buttonsDown[e.button] = true
        buttonsPressed[e.button] = true
    elseif e.type == "mousereleased" then
        buttonsDown[e.button] = nil
    end
end

function juno.mouse.reset()
    for k in pairs(buttonsPressed) do
        buttonsPressed[k] = nil
    end
end

function juno.mouse.isDown(...)
    for i = 1, select("#", ...) do
      local b = select(i, ...)
        if buttonsDown[b] then
            return true
        end
    end
    return false
end

function juno.mouse.wasPressed(...)
    for i = 1, select("#", ...) do
        local b = select(i, ...)
        if buttonsPressed[b] then
            return true
        end
    end
    return false
end