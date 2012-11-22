linux : src/main.c
        gcc -o bin/main src/main.c \
        -ISDL/linux -LSDL/linux -lSDLmain -lSDL \
        -lX11 -lXi -lXmu -lGL -lpthread \
        -Ichipmunk -Lchipmunk/linux -lChipmunk
bin/% : src/%.c
        gcc -o $@ $< \
        -I SDL -LSDL -lSDLmain -lSDL \
        -framework cocoa -framework OpenGL \
        -I chipmunk -Lchipmunk -lChipmunk

#gcc src/*.c src/constraints/*.c -Iinclude/chipmunk -o libChipmunk.a -std=c99 -lm -shared -fPIC

