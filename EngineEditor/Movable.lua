

function init()
    
    print("init movable")
    local l = Light.new()
    l.intensity = 0.8
    l.attenuationRadius = 10
    l.color = Vec4.new(1, 0.4, 0.7, 1.0)

    entity.light = l

    timer = 0.0


    local suzanneList = scene:findEntitiesByName("Suzanne.dae")

    firstSuzanne = suzanneList[2]

    for k,v in ipairs(suzanneList) do
        v.parent = entity
    end

    suzanneList[1].parent = nil

   -- setScene(otherScene)

    

end


function update(dt)
    timer = timer + dt

    --entity.light.color = Vec4.new(entity.transform.position / 10.0)

    if timer > 5 then
        timer = 0.0
        print("ID moved", firstSuzanne.id)
        firstSuzanne.transform.position.x = firstSuzanne.transform.position.x + 1
    end
    entity.transform.rotation:rotate(dt * 45, Vec3.new(0, 1, 0))

end
