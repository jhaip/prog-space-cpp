cleanup("9")
when({"$ clock time is $time"}, function (result)
    local v = math.sin(tonumber(result["time"])*0.01)*100
    -- print(v)
    claim("#9 wish line from 0 0 to "..v.." 100")
end)