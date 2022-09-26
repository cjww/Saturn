function init()

    print("init inventory")
    inventory = {}

    assert(scene, "Scene was nil")

    local groundEntities = scene:findEntitiesByName("Ground")
    print(groundEntities, #groundEntities)
    local e = groundEntities[1]
    print(e.transform.position)

    addItem({name = "Axe"})
    inventory.lightRadius = 3.0
    
end

function update(dt)
    if entity.movable == nil then
        entity:addScript("Movable.lua")
    end
    
    if entity.light == nil then
        inventory.lightRadius = inventory.lightRadius + 1
        local light = Light.new()
        light.intensity = inventory.lightRadius
        entity.light = light

    end

    for _,item in ipairs(inventory) do
        
    end
    --entity.transform.position.y = entity.transform.position.y + dt
end

function addItem(item)
    table.insert(inventory, item)
end

