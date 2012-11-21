bin/% : src/%.c
	gcc -o $@ $< `sdl-config --cflags --libs` -framework OpenGL -I chipmunk chipmunk/libChipmunk.a