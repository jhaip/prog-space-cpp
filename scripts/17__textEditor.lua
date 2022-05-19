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
            cache = cache.."\\n"
        elseif key == "TAB" then
            cache = cache.."\\t"
        elseif key == "\"" then
            cache = cache.."\\\""
        elseif key ~= "RIGHT" and key ~= "LEFT" and key ~= "UP" and key ~= "DOWN" then
            cache = cache..key
        end
        claim("#17 text cache is", {"", cache})
    end
end)

register_when("17", {"$ text cache is $cache", "$ text cursor at $x $y"}, function (results)
    retract("#17 wish you had graphics %")
    for index, result in ipairs(results) do
        local graphics0 = string.format([===[{"type":"text","options":{"x":%s,"y":%s,"text":"%s"}}]===], 0, 50, result["cache"]) 
        local character_width = 20
        local character_height = 26
        local sx = tonumber(result["x"]) * character_width
        local sy = tonumber(result["y"]) * character_height
        local graphics1 = string.format([===[{"type":"rectangle","options":{"x":%s,"y":%s,"w":%s,"h":%s}}]===], sx, 50+sy, character_width, character_height) 
        local graphics = string.format([===[[%s,%s]]===], graphics0, graphics1)
        claim("#17 wish you had graphics", {"", graphics})
    end
end)