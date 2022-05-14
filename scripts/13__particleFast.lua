cleanup("13")

p = ""
for i = 0,200,1 do
    p = p.."20:"..tostring(1*i)..":"..(i*0.1)..":".."0.5:,"
end
claim("#13 particles "..p)

register_when("13", {"$ particles $p", "$ clock time is $t"}, function (results)
    retract("#13 %")
    for index, result in ipairs(results) do
        new_particles = ""
        local graphics = ""
        graphics = graphics..string.format([===[{"type":"rectangle","options":{"x":0,"y":0,"w":300,"h":300}}]===])
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
            -- if i > 0 then
            --     graphics = graphics..","
            -- end
            graphics = graphics..","
            graphics = graphics..string.format([===[{"type":"ellipse","options":{"x":%s,"y":%s,"w":10,"h":10}}]===], new_x, new_y)
            -- claim("#13 wish text "..i.." at "..tostring(new_x).." "..tostring(new_y))
            i = i + 1
        end
        graphics = string.format([===[[%s]]===], graphics)
        claim("#13 wish you had graphics "..graphics)
        claim("#13 particles "..new_particles)
    end
end)
