register_when("8", {"$ program $program at $x $y $ $ $ $ $ $", "$program is a $type card"}, function (results)
    -- calculate stack based on X/y order of cards
    retract("#8 state is %")
    for index, result in ipairs(results) do
        claim("#8 state is "..{"turtle", "spiral", "emitter"})
    end
end)

register_when("8", {"(you) state is $state", "$ time is $timeMs"}, function (results)
    -- todo
    retract("#8 turtle %")
    for index, result in ipairs(results) do
        claim("#8 turtle ".."1".." at "..x.." "..y)
    end
end)

register_when("8", {"(you) turtle $id at $x $y"}, function (results)
    -- todo
    retract("#8 draw %")
    for index, result in ipairs(results) do
        claim("#8 draw graphics "..graphics)
    end
end)