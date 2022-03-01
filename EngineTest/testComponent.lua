
--$component
Enemy = {
    health = 100,
    damage = 10,
    name = "Bob",

    onCreate = function()
        
    end,

    new = function()
        return setmetatable({}, self)
    end

}

return Enemy