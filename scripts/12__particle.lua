for i = 0,50,1 do
    claim("particle "..i.." at 20 "..tostring(1*i))
    claim("particle "..i.." velocity is "..i*0.1.." 0.5")
end

when({"$ particle $p at $x $y", "$ particle $p velocity is $vx $vy", "$ clock time is $t"}, function (result)
    retract("#12 %")
    for index, result in ipairs(results) do
        local new_x = tonumber(result["x"]) + tonumber(result["vx"])
        local new_y = tonumber(result["y"]) + tonumber(result["vy"])
        local new_vx = tonumber(result["vx"])
        local new_vy = tonumber(result["vy"])
        if new_x < 0 or new_x > 300 then
            new_vx = -1*new_vx
        end
        if new_y < 0 or new_y > 300 then
            new_vy = -1*new_vy
        end
        claim("particle "..result["p"].." velocity is "..tostring(new_vx).." "..tostring(new_vy))
        claim("particle "..result["p"].." at "..tostring(new_x).." "..tostring(new_y))
        local ill = Illumination.new()
        ill:text{x=new_x, y=new_y, text=result["p"]}
        claim("wish you had graphics", {"", tostring(ill)})
    end
end)
