# You will need SDL2 (http://www.libsdl.org):
# Linux:    apt-get install libsdl2-dev
# Mac OS X: brew install sdl2
# MSYS2:    pacman -S mingw-w64-i686-SDL
#
#CXX = g++
#CXX = clang++

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Linux) #LINUX
	LIBS = -lvdb -lGL -ldl `sdl2-config --libs`
	CXXFLAGS = -I../include/ `sdl2-config --cflags` -L../lib/ -Wall -Wformat
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	LIBS = -lvdb -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo `sdl2-config --libs`
	CXXFLAGS = -I../include/ -I/usr/local/include `sdl2-config --cflags` -L../lib/ -Wall -Wformat
endif

ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
   LIBS = -lvdb -lgdi32 -lopengl32 -limm32 `pkg-config --static --libs sdl2`
   CXXFLAGS = -I../include/ `pkg-config --cflags sdl2` -L../lib/ -Wall -Wformat
endif

all: test.cpp
	$(CXX) test.cpp $(CXXFLAGS) $(LIBS) -o $(EXE)
