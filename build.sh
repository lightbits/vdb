# This compiles vdb into a library (libvdb.a) in the directory ./lib
# Make sure you have SDL2 (https://www.libsdl.org/).
# For example (linux): sudo apt-get install libsdl2-dev

g++ -c -O2 -Werror -Wall src/vdb.cpp -Iinclude/vdb -Ilib/freetype/include -Iinclude `sdl2-config --cflags`
mkdir -p lib
ar rvs lib/libvdb.a vdb.o
rm vdb.o
