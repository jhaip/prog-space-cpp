when("30", {"$ program 30 is pointing at $other"}, function (results)
    retract("#30 %")
    for index, result in ipairs(results) do
        local ill = Illumination.new()
        ill:text{x=0,y=60,text="Pointing at #"..result["other"], size=20}
        claim("#30 wish you had graphics", {"", tostring(ill)})
    end
end)