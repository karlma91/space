SYS := $(firstword $(shell uname -s))
precode=
postcode=
os=0

gamefiles=src/*.c #src/main.c src/draw.c src/font.c src/space.c src/particles.c

ifeq ($(SYS),Linux) #linux
	os=1
	suffix=
	os_lib=-lX11 -lXi -lXmu -lpthread
	openGL=-lGL
	sdl=-ISDL/linux -LSDL/linux -lSDLmain -lSDL 
	chipmunk=-Ichipmunk/include -Lchipmunk/linux -lChipmunk
	sdlimage=-ISDL_image/linux -LSDL_image/linux -lSDL_image
else ifneq ($(findstring MINGW32_NT, $(SYS)),) #windows
	os=2
	suffix=.exe
	precode = windres resources/win/space.rc -o coff -o resources/win/space.res.o #prepare resource file
	os_lib=-lmingw32 -mwindows resources/win/space.res.o -g -Wall
	openGL=-lopengl32
	sdl=-ISDL/windows_mingw/include/SDL -LSDL/windows_mingw/lib -lSDLmain -lSDL
	#sdlttf=-ISDL_ttf -LSDL_ttf/windows -lSDL_ttf
	#sdlimage=-ISDL_image/win -LSDL_image/win -lSDL_image -llibpng15-15
	chipmunk=-Ichipmunk/include -Lchipmunk/windows -lChipmunk
else ifeq ($(SYS),Darwin) #mac
	os=3
	suffix=
	postcode = rm -rf bin/space.app/ && \
		cp -R resources/mac/space.app/ bin/space.app && \
		cp bin/main bin/space.app/Contents/MacOS && \
		install_name_tool -change /opt/local/lib/libSDL-1.2.0.dylib @executable_path/libSDL.dylib bin/space.app/Contents/MacOS/main
	os_lib=-framework cocoa
	openGL=-framework OpenGL
	sdl=-ISDL/mac -LSDL/mac -lSDLmain -lSDL
	sdlimage=-framework SDL_image -Ilib/SDL_image
	chipmunk=-Ichipmunk/include -Lchipmunk/mac -lChipmunk
endif

all : 
	make build
	make run

build : main

main : src/main.c src/*.h
	$(precode)
	gcc -o bin/main$(suffix) $(gamefiles) $(os_lib) $(openGL) $(sdl) $(sdlimage) $(chipmunk) -Wall -g
	$(postcode)
#mingw and linux
#gcc src/*.c src/constraints/*.c -Iinclude/chipmunk -o libChipmunk.a -std=c99 -lm -shared -fPIC

run :
	if [ $(os) == 1 ]; then \
		clear; \
		bin/main; \
	elif [ $(os) == 2 ]; then \
		cls; \
		bin/main.exe; \
	elif [ $(os) == 3 ]; then \
		clear; \
		bin/space.app/Contents/MacOS/main; \
	else \
		clear; \
		echo "OS not recognized"; \
	fi