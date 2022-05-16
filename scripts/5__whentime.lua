register_when("5", {"$ time is $time"}, function (results)
    retract("#5 %")
    for index, result in ipairs(results) do
        claim("#5 I heard the time is "..results["time"])
        local graphics = string.format([===[[{"type":"text","options":{"x":%s,"y":%s,"text":"heard:%s"}}]]===], 0, 40, result["time"])
        claim("#5 wish you had graphics "..graphics)
    end
end)