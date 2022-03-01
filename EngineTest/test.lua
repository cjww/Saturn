--$using model transform

print("Hello world ")

local number = 10

function init()
    t = entity.transform

    --s = entity.script

    s = script
    print(s)

    print("hello from entity " .. entity.name .. " : " .. entity.id)
    if s.number == nil then
        print("no number in script")
    else
        s.number = 42
    end
    print(number)


end

function update(dt)
    t.position.z = t.position.z - dt
    
    
end
