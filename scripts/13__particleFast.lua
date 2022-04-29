cleanup("13")

p = ""
for i = 0,500,1 do
    p = p.."200:"..tostring(1*i)..":"..i..":".."2:,"
end
claim("#13 particles "..p)

register_when("13", {"$ particles $p", "$ clock time is $t"}, function (result)
    retract("#13 particles $")
    retract("#13 wish text % at %")
    new_particles = ""
    i = 0;
    for particle_str in string.gmatch(result["p"], '([^,]+)') do
        p = {}
        for ipstr in string.gmatch(particle_str, '([^:]+)') do
            p[#p + 1] = ipstr
        end
        local x = tonumber(p[1])
        local y = tonumber(p[2])
        local vx = tonumber(p[3])
        local vy = tonumber(p[4])
        local new_x = x + vx
        local new_y = y + vy
        if new_x < 0 or new_x > 800 then
            vx = -1*vx
        end
        if new_y < 0 or new_y > 800 then
            vy = -1*vy
        end
        new_particles = new_particles..new_x..":"..new_y..":"..vx..":"..vy..":,"
        claim("#13 wish text "..i.." at "..tostring(new_x).." "..tostring(new_y))
        i = i + 1
    end
    claim("#13 particles "..new_particles)
end)
