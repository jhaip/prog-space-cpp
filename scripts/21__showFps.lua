when("21", {"$ fps is $fps"}, function (results)
    retract("#21 %")
    for index, result in ipairs(results) do
        local ill = Illumination.new()
        ill:text{x=0, y=0, text="FPS: "..result["fps"], color={255,255,0}}
        claim("#21 wish all had graphics ", {"", tostring(ill)})
    end
end)