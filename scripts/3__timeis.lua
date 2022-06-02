register_when("3", {"$ clock time is $t"}, function (results)
    retract("#3 %")
    for index, result in ipairs(results) do
        claim("#3 time is "..os.time())
        local ill = Illumination.new()
        ill:text{x=0, y=50, text="time is "..os.time()}
        ill:rectangle{x = 10*(os.time() % 10), y = 0, w = 100, h = 50,
                      fill={0,0,255,200}, stroke={255, 0, 0}, stroke_width=5}
        ill:line{x1=0, y1=0, x2=500, y2=500, thickness=5, color={255, 0, 0, 100}}
        claim("#3 wish you had graphics", {"", tostring(ill)})
    end
end)