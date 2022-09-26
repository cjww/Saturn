
timer = 0.0

function init()
    
    print("init movable")
    local l = Light.new()
    l.intensity = 100
    entity.light = l

    timer = 0.0

    
end


function update(dt)
    timer = timer + dt

    if timer > 5 then
        entity.light = nil
        timer = 0.0
    end
    entity.transform.rotation:rotate(dt * 45, Vec3.new(0, 1, 0))

end
