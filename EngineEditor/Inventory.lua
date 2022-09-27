function init()

    print("init inventory")
    inventory = {}

    assert(scene, "Scene was nil")

    addItem({name = "Axe"})
    inventory.lightRadius = 3.0
    
end

function update(dt)
    for _,item in ipairs(inventory) do
        
    end
    --entity.transform.position.y = entity.transform.position.y + dt
end

function addItem(item)
    table.insert(inventory, item)
end

