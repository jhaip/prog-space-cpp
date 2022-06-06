when("2", {"$ $ is a $animal", "$ $animal is red"}, function (results)
    -- print("I see a: "..result["animal"])
    retract("#2 %")
    for index, result in ipairs(results) do
        claim("#2 red animal seen")
    end
end)