local defaultFont = juno.Font.fromEmbedded()

local fontTexCache = {}
setmetatable(fontTexCache, { 
    __index = function(t, k) 
        fontTexCache[k] = {}
        return fontTexCache[k]
    end,
    __mode = "v",
})

local font = defaultFont

function juno.graphics.setFont(newFont)
    font = newFont or defaultFont
end

function juno.graphics.print(text, x, y, r, sx, sy, ox, oy)
    text = tostring(text)
    x = x or 0
    y = y or 0
    r = r or 0
    sx = sx or 1
    sy = sy or sx
    ox = ox or 0
    oy = oy or 0

    if text:find("\n") then
        -- Multi line
        local height = font:getHeight()
        for line in (text .. "\n"):gmatch("(.-)\n") do
            juno.graphics.print(line, x, y, r, sx, sy, ox, oy)
            y = y + height
        end
    else
        -- Single line
        local tex = fontTexCache[font][text]
        if not tex then
            tex = font:render(text)
            fontTexCache[font][text] = tex
        end
        juno.graphics.draw(tex, x, y, nil, r, sx, sy, ox, oy)
    end
end

-- Override juno.graphics.clear() to use _clearColor if available
local clear = juno.graphics.clear
function juno.graphics.clear(r, g, b, a)
    local c = juno.graphics._clearColor
    r = r or (c and c[1])
    g = g or (c and c[2])
    b = b or (c and c[3])
    clear(r, g, b, 1)
end

function juno.graphics.setClearColor(...)
    juno.graphics._clearColor = { ... }
end