cleanup("11")

claim("#11 count is 1")

register_when("11", {"$ program $id at $ $ $ $ $ $ $ $", "$ count is $count"}, function (result)
    local count = tonumber(result["count"]) + 1
    -- print("new count: "..count)
    cleanup("11")
    claim("#11 count is "..count)
end)
