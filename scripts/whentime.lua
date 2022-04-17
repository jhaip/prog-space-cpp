cleanup("5")
when({"$ time is $time"}, function (result)
    claim("#5 I heard the time is "..result["time"])
end)