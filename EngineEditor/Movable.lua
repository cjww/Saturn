
function init()
    
    local l = Light.new()
    l.intensity = 100
    entity.light = l

    timer = 0.0
    print("init movable")
    
end


function update(dt)
    timer = timer + dt

    if timer > 5 then
        entity.light = nil
    end
    entity.transform.rotation:rotate(dt * 45, Vec3.new(0, 1, 0))

end
