--$using model transform

print("Hello world ")


function start()
    t = entity:getTransform()
end

function update(dt)
    t.position.z = t.position.z - dt
    
    --print(t)
    
    
end
