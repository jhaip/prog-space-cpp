cleanup("14")
remove_subs("14")

claim("#14 wish text no-weather-yet at 50 250")

register_when("14", {"$ weather forecast $d F and $type"}, function (result)
    retract("#14 wish text $ at %")
    claim("#14 wish text "..result["type"].." at 50 250")
end)
