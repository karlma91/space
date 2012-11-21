bin/% : src/%.c
	gcc -o $@ $< \
	-I SDL -LSDL -lSDLmain -lSDL \
	-framework cocoa -framework OpenGL \
	-I chipmunk -Lchipmunk -lChipmunk