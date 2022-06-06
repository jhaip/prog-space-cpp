claim("#11 count is 1")

when("11", {"$ program $id at $ $ $ $ $ $ $ $", "$ count is $count"}, function (results)
    retract("#11 %")
    for index, result in ipairs(results) do
        local count = tonumber(result["count"]) + 1
        -- print("new count: "..count)
        claim("#11 count is "..count)
    end
end)
