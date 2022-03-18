--$using model transform


print("Hello world ")

function init()
    

    t = entity.transform
    assert(t == transform, "t and transform not same")
    --s = entity.script
    s = script
    assert(s.t == transform, "s.t and transform not same")

    print("hello from entity " .. entity.name .. " : " .. entity.id)
    
end

function update(dt)
    t.position.z = t.position.z - dt
end
