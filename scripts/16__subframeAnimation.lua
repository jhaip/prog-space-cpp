claim("current frame is 0 at 0")

when({"$ program $id at $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4"}, function (results)
    retract("#16 subframe %")
    for index, result in ipairs(results) do
        local id = tonumber(result["id"])
        local x1 = tonumber(result["x1"])
        local y1 = tonumber(result["y1"])
        local x2 = tonumber(result["x2"])
        local y2 = tonumber(result["y2"])
        local x3 = tonumber(result["x3"])
        local y3 = tonumber(result["y3"])
        local x4 = tonumber(result["x4"])
        local y4 = tonumber(result["y4"])
        local x = math.min(x1, x2, x3, x4) - 50
        local y = math.min(y1, y2, y3, y4) -100
        if id == 23 then
            claim("subframe 0 at "..x.." "..y)
        elseif id == 24 then
            claim("subframe 1 at "..x.." "..y)
        elseif id == 25 then
            claim("subframe 2 at "..x.." "..y)
        elseif id == 26 then
            claim("subframe 3 at "..x.." "..y)
        end
    end
end)

when({"$ clock time is $time", "#16 current frame is $i at $frametime", "#16 subframe $i at $x1 $y1"}, function (results)
    retract("#16 current frame %")
    retract("#16 wish %")
    if #results == 0 then
        claim("current frame is 0 at 0")
    end
    for index, result in ipairs(results) do
        local new_i = tonumber(result["i"])
        local x1 = tonumber(result["x1"])
        local y1 = tonumber(result["y1"])
        if ((tonumber(result["time"]) % 10 == 0) and (result["time"] ~= result["frametime"])) then
            new_i = (new_i + 1) % 4
            claim("current frame is "..new_i.." at "..result["time"])
        else
            claim("current frame is "..new_i.." at "..result["frametime"])
        end
        local ill = Illumination.new()
        ill:frame{x=0, y=50, clip_x=x1, clip_y=y1, clip_w=100, clip_h=100}
        ill:text{x=50, y=0, text=tostring(new_i)}
        claim("wish you had graphics", {"", tostring(ill)})
    end
end)
