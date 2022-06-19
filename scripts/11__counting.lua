claim("count is 1")

when({"$ program $id at $ $ $ $ $ $ $ $", "$ count is $count"}, function (results)
    retract("#11 %")
    for index, result in ipairs(results) do
        local count = tonumber(result["count"]) + 1
        -- print("new count: "..count)
        claim("count is "..count)
    end
end)
