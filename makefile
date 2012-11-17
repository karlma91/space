% : %.c
	gcc -o $@ $< `sdl-config --cflags --libs` -framework OpenGL -I Chipmunk-Mac Chipmunk-Mac/libChipmunk.a