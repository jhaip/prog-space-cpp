local ill = Illumination.new()
ill:image{x=0, y=60, scale=0.25, filepath="/Users/jhaip/Downloads/cat.jpeg"}
claim("#15 wish you had graphics", {"", tostring(ill)})