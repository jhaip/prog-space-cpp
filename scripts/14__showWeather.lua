cleanup("14")
remove_subs("14")

claim("#14 wish text no-weather-yet at 50 250")

register_when("14", {"$ weather forecast $d F and $type"}, function (results)
    retract("#14 wish text $ at %")
    for index, result in ipairs(results) do
        claim("#14 wish text "..result["type"].." at 50 250")
    end
end)
