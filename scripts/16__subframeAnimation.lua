claim("#16 current frame is 0 at 0")

register_when("16", {"$ program $id at $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4"}, function (results)
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
        if id == 5 then
            claim("#16 subframe 0 at "..x1.." "..y1)
        elseif id == 7 then
            claim("#16 subframe 1 at "..x1.." "..y1)
        elseif id == 10 then
            claim("#16 subframe 2 at "..x1.." "..y1)
        elseif id == 4 then
            claim("#16 subframe 3 at "..x1.." "..y1)
        end
    end
end)

register_when("16", {"$ clock time is $time", "#16 current frame is $i at $frametime", "#16 subframe $i at $x1 $y1"}, function (results)
    retract("#16 current frame %")
    retract("#16 wish %")
    if #results == 0 then
        claim("#16 current frame is 0 at 0")
    end
    for index, result in ipairs(results) do
        local new_i = tonumber(result["i"])
        local x1 = tonumber(result["x1"])
        local y1 = tonumber(result["y1"])
        if ((tonumber(result["time"]) % 10 == 0) and (result["time"] ~= result["frametime"])) then
            new_i = (new_i + 1) % 4
            claim("#16 current frame is "..new_i.." at "..result["time"])
        else
            claim("#16 current frame is "..new_i.." at "..result["frametime"])
        end
        local ill = Illumination.new()
        ill:subframe(0, 50, 1, x1, y1+50, 120, 120)
        ill:text(50, 0, tostring(new_i))
        claim("#16 wish you had graphics", {"", tostring(ill)})
    end
end)
