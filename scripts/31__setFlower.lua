-- when({"$ time is $time"}, function (results)
--   retract("#31 %")
--   for index, result in ipairs(results) do
--       if tonumber(result["time"]) % 5 == 0 then
--           local percent = tonumber(result["time"]) % 100
--           claim("wish plant lamp was at "..percent.." percent")
--       end
--   end 
-- end)

-- when({"$ weather forecast $temp F and $weatherType"}, function (results)
--   retract("#31 %")
--   for index, result in ipairs(results) do
--       if result["weatherType"] == "clear-day" then
--           claim("wish plant lamp was at 100 percent")
--       else
--         claim("wish plant lamp was at 0 percent")
--       end
--   end 
-- end)

when({"$ time is $time"}, function (results)
  retract("#31 wish %")
  for index, result in ipairs(results) do
      if tonumber(result["time"]) % 10 == 0 then
          claim("wish currently Spotify song would be updated")
      end
      if tonumber(result["time"]) % 30 == 0 then
        claim("wish spotify:album:3l7JWewI3ZByxaT5BCgRx2 would be played on Spotify")
      end
  end 
end)