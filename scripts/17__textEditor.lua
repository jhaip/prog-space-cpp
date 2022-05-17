claim("#17 text cache is -")

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
        else
            cache = cache..key
        end
        claim("#17 text cache is \""..cache.."\"")
    end
end)

register_when("17", {"$ text cache is $cache"}, function (results)
    retract("#17 wish you had graphics %")
    for index, result in ipairs(results) do
        local graphics = string.format([===[[{"type":"text","options":{"x":%s,"y":%s,"text":"%s"}}]]===], 0, 50, result["cache"])
        claim("#17 wish you had graphics "..string.format("%q", graphics))
    end
end)