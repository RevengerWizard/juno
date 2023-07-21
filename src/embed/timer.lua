local last = 0
local delta = 0
local average = 0
local avgTimer = 0
local avgAcc = 1
local avgCount = 1

function juno.timer.step()
    local now = juno.timer.getTime()
    if last == 0 then last = now end
    delta = now - last
    last = now
    avgTimer = avgTimer - delta
    avgAcc = avgAcc + delta
    avgCount = avgCount + 1
    if avgTimer <= 0 then
        average = avgAcc / avgCount
        avgTimer = avgTimer + 1
        avgCount = 0
        avgAcc = 0
    end
end

function juno.timer.getDelta()
    return delta
end

function juno.timer.getAverage()
    return average
end

function juno.timer.getFps()
    return math.floor(1 / average + .5)
end