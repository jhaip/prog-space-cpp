cleanup("2")
when({"$ $ is a $animal", "$ $animal is red"}, function (result)
    -- print("I see a: "..result["animal"])
    claim("#2 red animal seen")
end)