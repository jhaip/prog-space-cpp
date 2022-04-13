when({"$ is a $animal", "$animal is red"}, function (result)
    print("I see a: "..result["animal"])
    claim("red animal seen")
end)