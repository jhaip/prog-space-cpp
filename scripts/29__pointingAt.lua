when("29", {"$ program $id at $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4"}, function (results)

    function get_paper_center(c)
        local x = (c[0]["x"] + c[1]["x"] + c[2]["x"] + c[3]["x"]) * 0.25
        local y = (c[0]["y"] + c[1]["y"] + c[2]["y"] + c[3]["y"]) * 0.25
        return {x=x, y=y}
    end

    function move_along_vector(amount, vector)
        local size = math.sqrt(vector["x"]*vector["x"] + vector["y"]*vector["y"])
        local C = 1.0
        if size ~= 0 then
            C = 1.0 * amount / size
        end
        return {x=C * vector["x"], y=C * vector["y"]}
    end

    function add_vec(vec1, vec2)
        return {x=vec1["x"] + vec2["x"], y=vec1["y"] + vec2["y"]}
    end

    function diff_vec(vec1, vec2)
        return {x=vec1["x"] - vec2["x"], y=vec1["y"] - vec2["y"]}
    end

    function scale_vec(vec, scale)
        return {x=vec["x"] * scale, y=vec["y"] * scale}
    end

    function get_paper_wisker(corners, direction, length)
        local center = get_paper_center(corners)
        local segment = nil
        if direction == 'right' then
            segment = {[0]=corners[1], corners[2]}
        elseif direction == 'down' then
            segment = {[0]=corners[2], corners[3]}
        elseif direction == 'left' then
            segment = {[0]=corners[3], corners[0]}
        else
            segment = {[0]=corners[0], corners[1]}
        end
        local segmentMiddle = add_vec(segment[1], scale_vec(diff_vec(segment[0], segment[1]), 0.5))
        local wiskerEnd = add_vec(segmentMiddle, move_along_vector(length, diff_vec(segmentMiddle, center)))
        return {[0]=segmentMiddle, wiskerEnd}
    end

    --Adapted from https://stackoverflow.com/questions/9043805/test-if-two-lines-intersect-javascript-function
    function intersects(v1,  v2,  v3,  v4)
        local det = (v2["x"] - v1["x"]) * (v4["y"] - v3["y"]) - (v4["x"] - v3["x"]) * (v2["y"] - v1["y"])
        if det == 0 then
            return false
        else
            _lambda = ((v4["y"] - v3["y"]) * (v4["x"] - v1["x"]) + (v3["x"] - v4["x"]) * (v4["y"] - v1["y"])) / det
            gamma = ((v1["y"] - v2["y"]) * (v4["x"] - v1["x"]) + (v2["x"] - v1["x"]) * (v4["y"] - v1["y"])) / det
            return (0 < _lambda and _lambda < 1) and (0 < gamma and gamma < 1)
        end
    end

    function get_paper_you_point_at(papers, you_id, my_paper, WISKER_LENGTH)
        local wisker = get_paper_wisker(my_paper["corners"], "up", WISKER_LENGTH)
        for id,paper in pairs(papers) do
            local corners = paper["corners"]
            if my_paper["seenByCamera"] == paper["seenByCamera"] then
                if (intersects(wisker[0], wisker[1], corners[0], corners[1]) or
                    intersects(wisker[0], wisker[1], corners[1], corners[2]) or
                    intersects(wisker[0], wisker[1], corners[2], corners[3]) or
                    intersects(wisker[0], wisker[1], corners[3], corners[0])) then
                    return id
                end
            end
        end
        return nil
    end

    retract("#29 %")
    local programs = {}
    for index, result in ipairs(results) do
        local x1 = tonumber(result["x1"])
        local y1 = tonumber(result["y1"])
        local x2 = tonumber(result["x2"])
        local y2 = tonumber(result["y2"])
        local x3 = tonumber(result["x3"])
        local y3 = tonumber(result["y3"])
        local x4 = tonumber(result["x4"])
        local y4 = tonumber(result["y4"])
        local tl = {x=x1,y=y1}
        local tr = {x=x2,y=y2}
        local br = {x=x3,y=y3}
        local bl = {x=x4,y=y4}
        local tr1 = add_vec(tl, scale_vec(diff_vec(tr, tl), 2))
        local br1 = add_vec(br, add_vec(scale_vec(diff_vec(tr, tl), 2), scale_vec(diff_vec(bl, tl), 3)))
        local bl1 = add_vec(bl, scale_vec(diff_vec(bl, tl), 3))
        programs[result["id"]] = {seenByCamera=1, corners={[0]=tl, tr1, br1, bl1}}
    end
    local WISKER_LENGTH = 60
    local ill = Illumination.new()
    for id,data in pairs(programs) do
        local wisker = get_paper_wisker(data["corners"], "up", WISKER_LENGTH)
        ill:line{x1=wisker[0]["x"], y1=wisker[0]["y"], x2=wisker[1]["x"], y2=wisker[1]["y"], color={0, 255, 0}, thickness=1}
        local other_paper = get_paper_you_point_at(programs, id, data, WISKER_LENGTH)
        if (other_paper ~= nil) then
            claim("#29 program "..id.." is pointing at "..other_paper)
            ill:line{x1=wisker[0]["x"], y1=wisker[0]["y"], x2=wisker[1]["x"], y2=wisker[1]["y"], color={255, 0, 0}, thickness=2}
        end
    end
    claim("#29 wish all had graphics", {"", tostring(ill)})
end)