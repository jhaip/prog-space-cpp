register_when("10", {"$ program $id at $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4"}, function (results)
    retract("#10 %")
    for index, result in ipairs(results) do
        local x1 = tonumber(result["x1"])
        local y1 = tonumber(result["y1"])
        local x2 = tonumber(result["x2"])
        local y2 = tonumber(result["y2"])
        local x3 = tonumber(result["x3"])
        local y3 = tonumber(result["y3"])
        local x4 = tonumber(result["x4"])
        local y4 = tonumber(result["y4"])
        line1 = string.format([===[{"type":"line","options":[%s,%s,%s,%s]}]===], x1, y1, x2, y2)
        line2 = string.format([===[{"type":"line","options":[%s,%s,%s,%s]}]===], x2, y2, x3, y3)
        line3 = string.format([===[{"type":"line","options":[%s,%s,%s,%s]}]===], x3, y3, x4, y4)
        line4 = string.format([===[{"type":"line","options":[%s,%s,%s,%s]}]===], x4, y4, x1, y1)
        graphics = "["..line1..","..line2..","..line3..","..line4.."]"
        claim("#10 wish all had graphics "..graphics)
    end
end)