width = 200
height = 120

function juno.load(dt)
    particle = juno.Buffer.fromFile("data/image/particle.png")
    particles = {}
    for i = 0, 200 do
        table.insert(particles, {
            x = 0,
            y = 0,
            vx = 0,
            vy = 0,
            t = 0,
            a = 0,
            s = math.random(),
        })
    end
end

function juno.update(dt)
    for i, p in ipairs(particles) do
        p.x = p.x + p.vx * dt
        p.y = p.y + p.vy * dt
        if p.t > 0 then
            p.t = p.t - dt
        else
            p.a = p.a - dt * 3
            if p.a < 0 then
                local r = math.random() * math.pi * 2
                p.x = math.cos(r) * 20 
                p.y = math.sin(r) * 20
                p.vx = (1 - math.random() * 2) * 90
                p.vy = (1 - math.random() * 2) * 90
                p.t = math.random() * 1
                p.a = 1
            end
        end
    end
end

function juno.draw()
    juno.graphics.clear(0, 0, 0, 255)
    juno.graphics.setBlend("add")
    juno.graphics.setColor(51, 102, 255)
    for i, p in ipairs(particles) do
        juno.graphics.setAlpha(p.a * 255)
        juno.graphics.draw(particle, width / 2 + p.x, height / 2 + p.y,
                      nil, 0, p.s, p.s, 16, 16)
    end
end