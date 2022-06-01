register_when("3", {"$ clock time is $t"}, function (results)
    retract("#3 %")
    for index, result in ipairs(results) do
        claim("#3 time is "..os.time())
        local ill = Illumination.new()
        ill:text(0, 50, "time is "..os.time())
        ill:rectangle{x = 10*(os.time() % 10), y = 0, w = 100, h = 50,
                      fill={0,0,255,200}, stroke={255, 0, 0}, stroke_width=5}
        claim("#3 wish you had graphics", {"", tostring(ill)})
    end
end)