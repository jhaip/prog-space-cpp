claim("#17 text cache is -")
claim("#17 text cursor at 0 0")

register_when("17", {"$ keyboard typed key $key", "$ text cursor at $x $y"}, function (results)
    for index, result in ipairs(results) do
        retract("#17 text cursor at %")
        local x = tonumber(result["x"])
        local y = tonumber(result["y"])
        local key = result["key"]
        if key == "RIGHT" then
            x = x + 1
        elseif key == "LEFT" then
            if x > 0 then
                x = x - 1
            end
        elseif key == "UP" then
            if y > 0 then
                y = y - 1
            end
        elseif key == "DOWN" then
            y = y + 1
        end
        claim("#17 text cursor at "..x.." "..y)
    end
end)

register_when("17", {"$ keyboard typed key $key", "$ text cache is $cache"}, function (results)
    for index, result in ipairs(results) do
        retract("#17 text cache is %")
        local cache = result["cache"]
        local key = result["key"]
        if key == "BACKSPACE" then
            cache = cache:sub(1, -2)
        elseif key == "SPACE" then
            cache = cache.." "
        elseif key == "ENTER" then
            cache = cache.."\n"
        elseif key == "TAB" then
            cache = cache.."\t"
        elseif key == "\"" then
            cache = cache.."\""
        elseif key ~= "RIGHT" and key ~= "LEFT" and key ~= "UP" and key ~= "DOWN" then
            cache = cache..key
        end
        claim("#17 text cache is", {"", cache})
    end
end)

register_when("17", {"$ text cache is $cache", "$ text cursor at $x $y"}, function (results)
    retract("#17 wish you had graphics %")
    for index, result in ipairs(results) do
        local ill = Illumination.new()
        ill:text(0, 150, result["cache"])
        local w = ""
        for i = 0,tonumber(result["y"])-1,1
        do 
            w = w.."\n"
        end
        for i = 0,tonumber(result["x"])-1,1
        do 
            w = w.." "
        end
        w = w.."█"
        ill:text(0, 150, w, {255, 255, 255, 150})
        claim("#17 wish you had graphics", {"", tostring(ill)})
    end
end)