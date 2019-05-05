# This compiles vdb into a library (libvdb.a) in the directory ./lib
# Make sure you have SDL2 (https://www.libsdl.org/).
# For example (linux): sudo apt-get install libsdl2-dev

g++ -std=c++11 -c -O2 -fPIC -Werror -Wall src/vdb.cpp -Iinclude/vdb -Isrc/freetype/include -Iinclude `sdl2-config --cflags`
mkdir -p lib
ar rvs lib/libvdb.a vdb.o
rm vdb.o
