--$using model transform

function count()
    if(i == nil) then
        i = 0
    end
    i = i + 1
end

function init()

    timer = 0.0
    t = entity.transform
    --assert(t == transform, "t and transform not same")
    --s = entity.script
    s = script
    --assert(s.t == transform, "s.t and transform not same")

    --print("hello from entity " .. entity.name .. " : " .. entity.id)
    originalpos = transform.position.y
    --transform.rotation.x = -90;
    --print(type(transform.position))

    count()
    print("system2: ", transform, _G, _ENV)
end

function update(dt)
    timer = timer + dt
    --transform.rotation.y = transform.rotation.y + dt * 10;
    transform.rotation:rotate(dt * 10, Vec3.new(0, 1, 0));
    --transform.position.y = originalpos + math.sin(timer * 10)
end
