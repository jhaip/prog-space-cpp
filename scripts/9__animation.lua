register_when("9", {"$ clock time is $time"}, function (results)
    retract("#9 %")
    for index, result in ipairs(results) do
        local dx = math.cos(tonumber(result["time"])*0.01)*50
        local dy = math.sin(tonumber(result["time"])*0.01)*50
        local ill = Illumination.new()
        ill:ellipse(50-50, 100-50, 100, 100)
        ill:line(50, 100, 50+dx, 100+dy)
        claim("#9 wish you had graphics ", {"", tostring(ill)})
    end
end)