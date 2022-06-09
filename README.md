# prog-space-cpp
Experimental c++ and lua version of a programmable-space

*Messy sfeMovie stuff that I want to make better:*
First download build and install sfeMovie. Then modify CMakeLists.txt so that SFEMOVIE_ROOT points to the build location.
Then symlink the library: `sudo ln -s ~/Personal/sfeMovie/bin/sfeMovie.framework /Library/Frameworks/sfeMovie.framework`.

```
$ mkdir build && cd build

$ conan install .. --build=missing

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