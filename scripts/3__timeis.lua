register_when("3", {"$ clock time is $t"}, function (results)
    retract("#3 %")
    for index, result in ipairs(results) do
        claim("#3 time is "..os.time())
        local graphics = string.format([===[[{"type":"text","options":{"x":%s,"y":%s,"text":"%s"}}]]===], 0, 50, "time-is-"..os.time())
        claim("#3 wish you had graphics "..graphics)
    end
end)