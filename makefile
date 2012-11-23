windows: src/main.c
	gcc -o bin\winodws/main.exe src/main.c \
	-lmingw32 -lopengl32 -mwindows \
	-ISDL/windows_mingw/include/SDL -LSDL/windows_mingw/lib -lSDLmain -lSDL  \
	-Ichipmunk/include -Lchipmunk/windows -lChipmunk 

linux : src/main.c
	gcc -o bin/linux/main src/main.c \
	-ISDL/linux -LSDL/linux -lSDLmain -lSDL \
	-lX11 -lXi -lXmu -lGL -lpthread \
	-Ichipmunk/include -Lchipmunk/linux -lChipmunk

#mac
bin/% : src/%.c
	gcc -o $@ $< \
	-ISDL/mac -LSDL/mac -lSDLmain -lSDL \
	-framework cocoa -framework OpenGL \
	-Ichipmunk/include -Lchipmunk/mac -lChipmunk

#mingw and linux
#gcc src/*.c src/constraints/*.c -Iinclude/chipmunk -o libChipmunk.a -std=c99 -lm -shared -fPIC

