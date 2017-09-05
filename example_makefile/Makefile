all:
	g++ -I../src main.cpp vdb.o `sdl2-config --cflags --libs` -lGL -o app

vdb.o:
	g++ -c -O2 vdb.cpp -o vdb.o `sdl2-config --cflags`
