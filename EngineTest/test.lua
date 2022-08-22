--$using model transform

function init()

    timer = 0.0
    t = entity.transform
    assert(t == transform, "t and transform not same")
    --s = entity.script
    s = script
    assert(s.t == transform, "s.t and transform not same")

    --print("hello from entity " .. entity.name .. " : " .. entity.id)
    originalpos = transform.position.y
    transform.rotation.x = -90;
end

function update(dt)
    timer = timer + dt;
    transform.rotation.z = transform.rotation.z + dt * 10;
    --transform.position.y = originalpos + math.sin(timer * 10)
end
