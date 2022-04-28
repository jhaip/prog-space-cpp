cleanup("12")

for i = 0,50,1 do
    claim("#12 particle "..i.." at 200 "..tostring(10*i))
    claim("#12 particle "..i.." velocity is "..i.." 2")
end

register_when("12", {"$ particle $p at $x $y", "$ particle $p velocity is $vx $vy", "$ clock time is $t"}, function (result)
    local new_x = tonumber(result["x"]) + tonumber(result["vx"])
    local new_y = tonumber(result["y"]) + tonumber(result["vy"])
    retract("#12 particle "..result["p"].." %")
    retract("#12 wish text "..result["p"].." at %")
    local new_vx = tonumber(result["vx"])
    local new_vy = tonumber(result["vy"])
    if new_x < 0 or new_x > 800 then
        new_vx = -1*new_vx
    end
    if new_y < 0 or new_y > 800 then
        new_vy = -1*new_vy
    end
    claim("#12 particle "..result["p"].." velocity is "..tostring(new_vx).." "..tostring(new_vy))
    claim("#12 particle "..result["p"].." at "..tostring(new_x).." "..tostring(new_y))
    claim("#12 wish text "..result["p"].." at "..tostring(new_x).." "..tostring(new_y))
end)
