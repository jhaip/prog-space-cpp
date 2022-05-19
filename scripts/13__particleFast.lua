p = ""
for i = 0,200,1 do
    p = p.."20:"..tostring(1*i)..":"..(i*0.1)..":".."0.5:,"
end
claim("#13 particles "..p)

register_when("13", {"$ particles $p", "$ clock time is $t"}, function (results)
    retract("#13 %")
    for index, result in ipairs(results) do
        new_particles = ""
        local ill = Illumination.new()
        ill:rectangle(0, 0, 300, 300)
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
            if new_x < 0 or new_x > 300 then
                vx = -1*vx
            end
            if new_y < 0 or new_y > 300 then
                vy = -1*vy
            end
            new_particles = new_particles..new_x..":"..new_y..":"..vx..":"..vy..":,"
            ill:ellipse(new_x, new_y, 10, 10)
            i = i + 1
        end
        claim("#13 wish you had graphics", {"", tostring(ill)})
        claim("#13 particles "..new_particles)
    end
end)
