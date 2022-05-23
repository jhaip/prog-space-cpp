claim("#17 text cache is -")
claim("#17 text cursor at 0 0")

register_when("17", {"$ 3 source code $code"}, function (results)
    for index, result in ipairs(results) do
        retract("#17 text cache is %")
        claim("#17 text cache is", {"", result["code"]})
    end
end)

register_when("17", {"$ keyboard typed key $key", "$ text cache is $cache", "$ text cursor at $x $y"}, function (results)
    for index, result in ipairs(results) do
        retract("#17 text cache is %")
        retract("#17 text cursor at %")
        local cache = result["cache"]
        local key = result["key"]
        local cursor_x = tonumber(result["x"])
        local cursor_y = tonumber(result["y"])
        local new_cursor_x = cursor_x
        local new_cursor_y = cursor_y

        local cursor_position_in_text = 0
        local n_newlines_seen = 0
        local offset_in_row = 0
        local newline = string.byte("\n")
        for i = 1, #cache do
            if cache:byte(i) == newline then
                n_newlines_seen = n_newlines_seen + 1
                offset_in_row = 0
            else
                offset_in_row = offset_in_row + 1
            end
            if cursor_y == n_newlines_seen and offset_in_row == cursor_x then
                cursor_position_in_text = i
            end
        end
        if cursor_position_in_text == 0 then
            cursor_position_in_text = #cache
        end
        if key == "BACKSPACE" then
            if cache:byte(cursor_position_in_text) == newline then
                new_cursor_y = new_cursor_y - 1
            else
                new_cursor_x = new_cursor_x - 1
            end
            cache = cache:sub(1, cursor_position_in_text-1) .. cache:sub(cursor_position_in_text+1)
        elseif key == "SPACE" then
            cache = cache:sub(1, cursor_position_in_text) .. " " .. cache:sub(cursor_position_in_text+1)
            new_cursor_x = new_cursor_x + 1
        elseif key == "ENTER" then
            cache = cache:sub(1, cursor_position_in_text) .. "\n" .. cache:sub(cursor_position_in_text+1)
            new_cursor_y = new_cursor_y + 1
            new_cursor_x = 0
        elseif key == "TAB" then
            cache = cache:sub(1, cursor_position_in_text) .. "\t" .. cache:sub(cursor_position_in_text+1)
            new_cursor_x = new_cursor_x + 1
        elseif key == "\"" then
            cache = cache:sub(1, cursor_position_in_text) .. "\"" .. cache:sub(cursor_position_in_text+1)
            new_cursor_x = new_cursor_x + 1
        elseif key == "CONTROL-p" then
            print("print me!")
        elseif key == "RIGHT" then
            new_cursor_x = cursor_x + 1
        elseif key == "LEFT" then
            if cursor_x > 0 then
                new_cursor_x = cursor_x - 1
            end
        elseif key == "UP" then
            if cursor_y > 0 then
                new_cursor_y = cursor_y - 1
            end
        elseif key == "DOWN" then
            new_cursor_y = cursor_y + 1
        else
            cache = cache:sub(1, cursor_position_in_text) .. key .. cache:sub(cursor_position_in_text+1)
            new_cursor_x = new_cursor_x + 1
        end
        claim("#17 text cache is", {"", cache})
        claim("#17 text cursor at "..new_cursor_x.." "..new_cursor_y)
    end
end)

register_when("17", {"$ text cache is $cache", "$ text cursor at $x $y"}, function (results)
    retract("#17 wish you had graphics %")
    for index, result in ipairs(results) do
        local ill = Illumination.new()
        ill:text(0, 150, result["cache"])
        local w = ""
        for i = 0,tonumber(result["y"])-1,1
        do 
            w = w.."\n"
        end
        for i = 0,tonumber(result["x"])-1,1
        do 
            w = w.." "
        end
        w = w.."█"
        ill:text(0, 150, w, {255, 255, 255, 150})
        claim("#17 wish you had graphics", {"", tostring(ill)})
    end
end)