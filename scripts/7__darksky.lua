when({"$ time is $time"}, function (results)
    retract("#7 %")
    for index, result in ipairs(results) do
        if tonumber(result["time"]) % 10 == 0 then
            print("making http request")
            darkSkyUrl = "https://api.darksky.net/forecast/ff4210a6ee0e933946c817939138eb1f/42.3601,-71.0589?exclude=minutely,hourly,alerts,flags"
            -- http_request({"http request to "..darkSkyUrl.." results are $results"}, function (httpResults)
            --     print("got results")
            --     print(httpResults)
            --     local degrees = httpResults["currently"]["temperature"]
            --     local weatherType = httpResults["currently"]["icon"]
            --     cleanup("7")
            --     claim("weather forecast "..degrees.." F and "..weatherType)
            -- end)
        end
    end
end)

-- print("making http request")
-- darkSkyUrl = "https://api.darksky.net/forecast/ff4210a6ee0e933946c817939138eb1f/42.3601,-71.0589?exclude=minutely,hourly,alerts,flags"
-- http_request({"http request to "..darkSkyUrl.." results are $results"}, function (httpResults)
--     print("got results")
--     print(httpResults)
--     local degrees = httpResults["currently"]["temperature"]
--     local weatherType = httpResults["currently"]["icon"]
--     claim("weather forecast "..degrees.." F and "..weatherType)
-- end)