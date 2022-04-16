# prog-space-cpp
Experimental c++ and lua version of a programmable-space

```
$ mkdir build && cd build

$ conan install ..

(win)
$ cmake .. -G "Visual Studio 16"
$ cmake --build . --config Release

(linux, mac)
$ cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
$ cmake --build .
```

Running, from project root:
```
./build/bin/ProgSpace
```