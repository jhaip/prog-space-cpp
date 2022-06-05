function drawAruco(startx, starty, size, code)
    local ill = Illumination.new()
    local white={150,150,150}
    ill:rectangle{x=startx, y=starty, w=size*10, h=size*10, fill=white, stroke_width=0}
    ill:rectangle{x=startx+size, y=starty+size, w=size*8, h=size*8, fill={0,0,0}, stroke_width=0}
    for key, value in pairs(code) do
        local y = starty + size*2 + (key-1)*size
        for xx, value2 in pairs(value) do
            local x = startx + size*2 + (xx-1)*size
            if value2 > 0 then
                ill:rectangle{x=x, y=y, w=size, h=size, fill=white, stroke_width=0}
            else
                ill:rectangle{x=x, y=y, w=size, h=size, fill={0,0,0}, stroke_width=0}
            end
        end
    end
    claim("#20 wish all had graphics ", {"", tostring(ill)})
end

drawAruco(0, 0, 10, {
{1,1,1,0,0,0},
{0,1,1,0,0,1},
{1,0,1,0,1,0},
{0,1,0,0,0,0},
{1,0,1,0,0,1},
{0,1,0,0,1,0}
}) -- 990

drawAruco(1920-10*10, 0, 10, {
{1,1,1,0,0,1},
{1,1,1,1,1,1},
{0,0,1,0,1,1},
{0,0,0,0,0,0},
{1,1,1,1,1,0},
{1,0,1,0,0,1}
}) -- 991

drawAruco(1920-10*10, 1080-10*10, 10, {
{1,1,1,0,1,1},
{1,0,1,0,1,0},
{1,1,0,1,1,0},
{1,1,1,1,1,0},
{1,0,0,0,0,0},
{1,1,1,0,0,0}
}) -- 992

drawAruco(0, 1080-10*10, 10, {
{1,1,1,1,0,0},
{0,0,0,0,0,1},
{1,1,0,0,1,1},
{1,1,0,0,1,0},
{0,1,1,1,1,1},
{0,0,0,0,0,1}
})