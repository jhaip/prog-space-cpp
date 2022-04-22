cleanup("10")
when({"$ program $id at $x $y"}, function (result)
    local x1 = (tonumber(result["x"])*0.5)
    local y1 = (tonumber(result["y"])*0.5)
    local x2 = x1+10
    local y2 = y1+10
    -- print(v)
    claim("#10 wish line from "..x1.." "..y1.." to "..x2.." "..y1)
    claim("#10 wish line from "..x1.." "..y1.." to "..x1.." "..y2)
    claim("#10 wish frame at "..x1.." "..y1.." scale 0.2")
end)