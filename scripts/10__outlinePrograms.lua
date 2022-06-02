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
        local ill = Illumination.new()
        ill:line{x1=x1, y1=y1, x2=x2, y2=y2}
        -- ill:line{x1=x2, y1=y2, x2=x3, y2=y3}
        -- ill:line{x1=x3, y1=y3, x2=x4, y2=y4}
        -- ill:line{x1=x4, y1=y4, x2=x1, y2=y1}
        claim("#10 wish all had graphics", {"", tostring(ill)})
    end
end)