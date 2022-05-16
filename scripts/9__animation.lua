register_when("9", {"$ clock time is $time"}, function (results)
    retract("#9 %")
    for index, result in ipairs(results) do
        local dx = math.cos(tonumber(result["time"])*0.01)*50
        local dy = math.sin(tonumber(result["time"])*0.01)*50
        background = string.format([===[{"type":"ellipse","options":{"x":%s,"y":%s,"w":%s,"h":%s}}]===], 50-50, 100-50, 100, 100)
        line = string.format([===[{"type":"line","options":[%s,%s,%s,%s]}]===], 50, 100, 50+dx, 100+dy)
        graphics = "["..background..","..line.."]"
        claim("#9 wish you had graphics "..graphics)
    end
end)