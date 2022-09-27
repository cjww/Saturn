function init()

    otherScene = getScene("otherScene")
    local tommy = otherScene:createEntity("Tommy")
    tommy:addComponent("Transform")
    
    local manny = otherScene:createEntity("Manny")
    manny:addComponent("Transform")
    
    for i=1,3 do
        local sid = otherScene:createEntity("Sid")
        sid:addComponent("Transform")
    end    
    
    local diego = otherScene:createEntity("Diego")
    diego:addComponent("Transform")

    manny.parent = tommy
    diego.parent = manny

    setScene(otherScene)
end