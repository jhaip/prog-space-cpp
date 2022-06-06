when("4", {"$ $someone is a fox"}, function (results)
    retract("#4 %")
    for index, result in ipairs(results) do
        claim("#4 you see a fox")
        local ill = Illumination.new()
        ill:text{x=0, y=40, text="I see a fox!"}
        claim("#4 wish you had graphics ", {"", tostring(ill)})
    end
end)