cleanup("4")
register_when("4", {"$ $someone is a fox"}, function (results)
    retract("#4 %")
    for index, result in ipairs(results) do
        claim("#4 you see a fox")
    end
end)