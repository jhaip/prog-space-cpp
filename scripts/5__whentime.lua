cleanup("5")
register_when("5", {"$ time is $time"}, function (results)
    retract("#5 %")
    for index, result in ipairs(results) do
        claim("#5 I heard the time is "..results["time"])
        claim("#5 wish text heard at 0 30")
    end
end)