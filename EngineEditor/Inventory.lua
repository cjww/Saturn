
function init()

    inventory = {}

    addItem({name = "Axe"})

end

function update(dt)
    for _,item in ipairs(inventory) do
        
    end
    --entity.transform.position.y = entity.transform.position.y + dt
end

function addItem(item)
    table.insert(inventory, item)
end

