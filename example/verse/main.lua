width = 500
height = 500

function juno.load()
    elapsed = 983
    stars = {}
    for i = 1, 10000 do
        local x = {
        s = math.random() * .5,
        r = math.random() * 200,
        z = math.random() * 1
        }
        table.insert(stars, x)
    end
end

function juno.update(dt)
    elapsed = elapsed + dt
end

function juno.draw()
    juno.graphics.setBlend("add")
    juno.graphics.setColor(102, 102, 178)
    for i, x in ipairs(stars) do
        local px = math.cos(x.s * elapsed + .3) * x.r
        local py = math.cos(x.s * elapsed + 1) * x.r
        px = px * x.z
        py = py * x.z
        juno.graphics.pixel(width / 2 + px, height / 2 + py)
    end
end