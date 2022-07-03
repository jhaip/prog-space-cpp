when({"$ time is $time"}, function (results)
    retract("#7 wish %")
    for index, result in ipairs(results) do
        if tonumber(result["time"]) % 60 == 0 then
            print("making http request")
            darkSkyUrl = "https://api.darksky.net/forecast/ff4210a6ee0e933946c817939138eb1f/42.3601,-71.0589?exclude=minutely,hourly,alerts,flags"
            claim("wish http request to "..darkSkyUrl.." with id darksky")
        end
    end 
end)

when({"$ http result id darksky $response"}, function (results)
    retract("#7 weather %")
    for index, result in ipairs(results) do
        print("got results")
        local jsonStr = result["response"]
        local r = convert_json(jsonStr)
        if (r["error"] ~= nil) then
            claim("weather forecast error")
        else
            local degrees = r["currently"]["temperature"]
            local weatherType = r["currently"]["icon"]
            retract("#7 %")
            claim("weather forecast "..degrees.." F and "..weatherType)
        end
    end 
end)