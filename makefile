SYS := $(firstword $(shell uname -s))

ifeq ($(SYS),Linux) #linux
	suffix=
	os_lib=-lX11 -lXi -lXmu -lpthread
	openGL=-lGL
	sdl=-ISDL/linux -LSDL/linux -lSDLmain -lSDL 
	chipmunk=-Ichipmunk/include -Lchipmunk/linux -lChipmunk
else ifneq ($(findstring MINGW32_NT, $(SYS)),) #windows
	suffix=.exe
	os_lib=-lmingw32 -mwindows
	openGL=-lopengl32
	sdl=-ISDL/windows_mingw/include/SDL -LSDL/windows_mingw/lib -lSDLmain -lSDL
	chipmunk=-Ichipmunk/include -Lchipmunk/windows -lChipmunk
else ifeq ($(SYS),Darwin) #mac
	suffix=
	os_lib=-framework cocoa
	openGL=-framework OpenGL
	sdl=-ISDL/mac -LSDL/mac -lSDLmain -lSDL
	chipmunk=-Ichipmunk/include -Lchipmunk/mac -lChipmunk
endif

main : src/main.c #$(addsuffix $(suffix),bin/main)
	gcc -o bin/main$(suffix) src/main.c $(os_lib) $(openGL) $(sdl) $(chipmunk)
	
#mingw and linux
#gcc src/*.c src/constraints/*.c -Iinclude/chipmunk -o libChipmunk.a -std=c99 -lm -shared -fPIC

