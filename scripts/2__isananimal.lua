when({"$ $ is a $animal", "$ $animal is red"}, function (results)
    -- print("I see a: "..result["animal"])
    retract("#2 %")
    for index, result in ipairs(results) do
        claim("red animal seen")
    end
end)