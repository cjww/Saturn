--$using model transform
local timer = 0.0;
local originalpos;
function onStart()

    print("hello from entity " .. this_entity.name .. " : " .. this_entity.id)
    
    originalpos = this_entity.transform.position.y;
end

function onUpdate(dt)

    timer = timer + dt
    
    
    this_entity.transform.rotation:rotate(dt * 50, Vec3.new(0, 1, 0));
    --entity.transform.position.y = originalpos + math.sin(timer * 10)
end
