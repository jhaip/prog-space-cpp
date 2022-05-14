cleanup("10")
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
        -- print(v)
        claim("#10 wish line from "..x1.." "..y1.." to "..x2.." "..y2)
        claim("#10 wish line from "..x2.." "..y2.." to "..x3.." "..y3)
        claim("#10 wish line from "..x3.." "..y3.." to "..x4.." "..y4)
        claim("#10 wish line from "..x4.." "..y4.." to "..x1.." "..y1)
        -- claim("#10 wish frame at "..x1.." "..y1.." scale 0.2")

        -- local graphics = string.format([===[[{"type":"rectangle","options":{"x":%s,"y":%s,"w":100,"h":100}}]]===], x1, y1)
        -- claim("#10 wish graphics "..graphics)
        -- graphics = string.format([===[[{"type":"ellipse","options":{"x":%s,"y":%s,"w":100,"h":100}}]]===], x1+100, y1)
        -- claim("#10 wish graphics "..graphics)
        -- graphics = string.format([===[[{"type":"line","options":[%s,%s,%s,%s]}]]===], x1-100, y1, x1-50, y1+50)
        -- claim("#10 wish graphics "..graphics)
        -- graphics = string.format([===[[{"type":"text","options":{"x":%s,"y":%s,"text":"%s"}}]]===], x1, y1+100, "hello-world")
        -- claim("#10 wish graphics "..graphics)
    end
end)