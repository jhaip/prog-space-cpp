local color = {255, 255, 0}
claim("#18 wish RGB light strand is color "..color[1].." "..color[2].." "..color[3])
local ill = Illumination.new()
ill:rectangle(0, 60, 100, 120, {color[1], color[2], color[3], 255})
claim("#18 wish you had graphics ", {"", tostring(ill)})