local color = {0, 255, 255}
claim("wish RGB light strand is color "..color[1].." "..color[2].." "..color[3])
local ill = Illumination.new()
ill:rectangle{x=0, y=60, w=100, h=120, fill={color[1], color[2], color[3]}}
claim("wish you had graphics ", {"", tostring(ill)})