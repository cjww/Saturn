--$using model transform

print("Hello world ")


function init()
    number = entity.id
    t = entity:getTransform()
    print("hello from entity " .. entity.id)
    print(t)
    local s = entity:getScript()

    print(s.number)

end

function update(dt)
    t.position.z = t.position.z - dt
    
    
end
