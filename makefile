% : %.c
	gcc -o $@ $^ `sdl-config --cflags --libs` -framework OpenGL