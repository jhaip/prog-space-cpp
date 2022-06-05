register_when("9", {"$ clock time is $time"}, function (results)
    retract("#9 %")
    for index, result in ipairs(results) do
        local dx = math.cos(tonumber(result["time"])*0.01)*50
        local dy = math.sin(tonumber(result["time"])*0.01)*50
        local ill = Illumination.new()
        ill:ellipse{x=50-50, y=100-50, w=100, h=100, fill={0,0,200,100}, stroke_width=0}
        ill:line{x1=50, y1=100, x2=50+dx, y2=100+dy}
        claim("#9 wish you had graphics ", {"", tostring(ill)})
    end
end)