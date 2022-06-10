when("4", {"$ $someone is a fox"}, function (results)
    retract("#4 %")
    for index, result in ipairs(results) do
        claim("#4 you see a fox")
        local ill = Illumination.new()
        ill:text{x=0, y=40, text="I see a fox!", size=20}
        claim("#4 wish you had graphics ", {"", tostring(ill)})
    end
    if #results == 0 then
        local ill = Illumination.new()
        ill:text{x=0, y=40, text="No fox.", size=20, color={255,200,200}}
        claim("#4 wish you had graphics ", {"", tostring(ill)})
    end
end)