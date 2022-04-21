cleanup("7")
when({"$ time is $t"}, function (result)
    if result["time"] % 60 == 0 then
        when({"http request to "..darkSkyUrl.." results are $results"}, function (httpResults)
            -- parse results
            claim("#7 weather forecast "..degrees.." F and "..weatherType)
        end)
    end
end)