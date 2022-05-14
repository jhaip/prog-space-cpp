cleanup("9")
register_when("9", {"$ clock time is $time"}, function (results)
    retract("#9 %")
    for index, result in ipairs(results) do
        local dx = math.cos(tonumber(result["time"])*0.01)*100
        local dy = math.sin(tonumber(result["time"])*0.01)*100
        -- print(v)
        claim("#9 wish line from 100 100 to "..(100+dx).." "..(100+dy))
    end
end)