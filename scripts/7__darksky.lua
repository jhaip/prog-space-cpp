cleanup("7")
remove_subs("7")
print("hello")
register_when("7", {"$ time is $time"}, function (result)
    print("running")
    if tonumber(result["time"]) % 10 == 0 then
        print("making http request")
        darkSkyUrl = "https://api.darksky.net/forecast/ff4210a6ee0e933946c817939138eb1f/42.3601,-71.0589?exclude=minutely,hourly,alerts,flags"
        http_request({"http request to "..darkSkyUrl.." results are $results"}, function (httpResults)
            print("got results")
            print(httpResults)
            local degrees = httpResults["currently"]["temperature"]
            local weatherType = httpResults["currently"]["icon"]
            cleanup("7")
            claim("#7 weather forecast "..degrees.." F and "..weatherType)
        end)
    end
end)

-- print("making http request")
-- darkSkyUrl = "https://api.darksky.net/forecast/ff4210a6ee0e933946c817939138eb1f/42.3601,-71.0589?exclude=minutely,hourly,alerts,flags"
-- http_request({"http request to "..darkSkyUrl.." results are $results"}, function (httpResults)
--     print("got results")
--     print(httpResults)
--     local degrees = httpResults["currently"]["temperature"]
--     local weatherType = httpResults["currently"]["icon"]
--     claim("#7 weather forecast "..degrees.." F and "..weatherType)
-- end)