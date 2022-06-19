local ill = Illumination.new()
ill:text{x=0, y=50, text="no weather yet"}
claim("wish you had graphics", {"", tostring(ill)})

when({"$ weather forecast $d F and $type"}, function (results)
    retract("#14 %")
    for index, result in ipairs(results) do
        local ill = Illumination.new()
        ill:text{x=0, y=50, text=result["type"]}
        claim("wish you had graphics", {"", tostring(ill)})
    end
end)
