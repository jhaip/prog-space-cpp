local graphics = string.format([===[[{"type":"text","options":{"x":%s,"y":%s,"text":"%s"}}]]===], 0, 50, "no-weather-yet")
claim("#14 wish you had graphics "..graphics)

register_when("14", {"$ weather forecast $d F and $type"}, function (results)
    retract("#14 %")
    for index, result in ipairs(results) do
        local ill = Illumination.new()
        ill:text{x=0, y=50, text=result["type"]}
        claim("#14 wish you had graphics", {"", tostring(ill)})
    end
end)
