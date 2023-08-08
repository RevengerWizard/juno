function juno.load()
    print("start load")

    -- Load the sprite sheet image
    spriteSheet = juno.Buffer.fromFile("wizard_red_staff.png")
    img = juno.Buffer.fromFile("wizard_red.png")

    -- Set the frame width and height
    frameWidth = 32
    frameHeight = 18

    -- Set the total number of frames
    totalFrames = 17

    -- Initialize the current frame
    currentFrame = 1

    -- Set the animation speed (in seconds)
    animationSpeed = 0.1

    -- Initialize the animation timer
    animationTimer = 0

    font = juno.Font.fromFile("nokia.ttf", 16)

    --music = juno.Data.fromFile("microfantasytheme.ogg")
    --source = juno.Source.fromData(music)

    --sound = juno.Source.fromData(juno.Data.fromFile("fire.ogg"))
    --source:setLoop(true)
    --source:setGain()
    --source:play(true)
    ---sound:play(true)
end

function juno.update(dt)
    -- Update the animation timer
    animationTimer = animationTimer + dt
    
    -- Change the current frame if the timer exceeds the animation speed
    if animationTimer >= animationSpeed then
        currentFrame = currentFrame + 1
        if currentFrame > totalFrames then
            currentFrame = 1
        end
        animationTimer = 0
    end
end

function juno.draw()
    juno.graphics.clear(41, 30, 49)
    juno.graphics.pixel(2, 2)
    juno.graphics.setFont(font)
    juno.graphics.print("hello world!", 60, 120, -1.570796, 2, 2)
    juno.graphics.setFont()
    juno.graphics.print("hello\npeople!\ncool\nstuff", 60, 150)
    juno.graphics.rectangle("line", 50, 50, 10, 30)

    juno.graphics.setColor(255, 0, 0)
    juno.graphics.circle("line", 100, 100, 15)
    juno.graphics.setColor()

    juno.graphics.setAlpha(160)
    juno.graphics.draw(img, 80, 80, {x = 0, y = 18, w = 32, h = 18}, -1.570796, 2, 2)
    juno.graphics.setAlpha()
    
    local currentColumn = (currentFrame - 1) % (spriteSheet:getWidth() / frameWidth)
    local currentRow = math.floor((currentFrame - 1) / (spriteSheet:getWidth() / frameWidth))
    
    -- Draw the current frame
    juno.graphics.draw(spriteSheet, 20, 20, {x = currentColumn * frameWidth, y = currentRow * frameHeight, w = frameWidth, h = frameHeight})
end

function juno.quit()
    print("quit")
end

function juno.keypressed(key)
    print("keypressed "..key)

    if key == "f" then
        source:pause()
    end

    if key == 'p' then
        juno.window.setSize(320 * 3, 180 * 3)
    end

    if key == 'escape' then
        print("quitting...")
        juno.event.quit()
    end
end

function juno.resize(w, h)
    print("resize  "..w..", "..h)
end

function juno.textinput(text)
    print("textinput  "..text)
end

function juno.keyreleased(key)
    print("keyreleased "..key)
end

function juno.mousemoved(x, y)
    print("mousemoved "..x..", "..y)
end

function juno.mousepressed(x, y, button)
    print("mousepressed "..button)
end

function juno.mousereleased(x, y, button)
    print("mousereleased "..button)
end

--function juno.joystickpressed(joystick, button)
--    print(joystick)
--    print("joystickpressed "..button)
--end

--function juno.joystickreleased(joystick, button)
--    print(joystick)
--    print("joystickreleased "..button)
--end

function juno.joystickaxis(joystick, axis, value)
    print("joystickaxis "..axis..", "..value)
end

function juno.joystickball(joystick, ball, x, y)
    print("joystickball "..ball..": "..x..", "..y)
end

function juno.gamepadpressed(joystick, button)
    --print(joystick)
    print("gamepadpressed "..button)
end

function juno.focus(f)
    if f then
        print("Window is focused.")
        text = "FOCUSED"
    else
        print("Window is not focused.")
        text = "UNFOCUSED"
    end
end

function juno.visible(v)
    print(v and "Window is visible!" or "Window is not visible!");
end