
function init()
    local list = scene:findEntitiesByName(entity.name)
    for k,v in ipairs(list) do
        v.parent = entity
    end
end