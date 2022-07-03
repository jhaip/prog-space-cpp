-- when({"$ time is $time"}, function (results)
--   retract("#31 %")
--   for index, result in ipairs(results) do
--       if tonumber(result["time"]) % 5 == 0 then
--           local percent = tonumber(result["time"]) % 100
--           claim("wish plant lamp was at "..percent.." percent")
--       end
--   end 
-- end)

when({"$ weather forecast $temp F and $weatherType"}, function (results)
  retract("#31 %")
  for index, result in ipairs(results) do
      if result["weatherType"] == "clear-day" then
          claim("wish plant lamp was at 100 percent")
      else
        claim("wish plant lamp was at 0 percent")
      end
  end 
end)