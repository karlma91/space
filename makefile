SYS := $(firstword $(shell uname -s))
precode=
postcode=

gamefiles=src/main.c src/draw.c

ifeq ($(SYS),Linux) #linux
	suffix=
	os_lib=-lX11 -lXi -lXmu -lpthread
	openGL=-lGL
	sdl=-ISDL/linux -LSDL/linux -lSDLmain -lSDL 
	chipmunk=-Ichipmunk/include -Lchipmunk/linux -lChipmunk
else ifneq ($(findstring MINGW32_NT, $(SYS)),) #windows
	suffix=.exe
	precode = windres resources/win/space.rc -o coff -o resources/win/space.res.o #prepare resource file
	os_lib=-lmingw32 -mwindows resources\win\space.res.o
	openGL=-lopengl32
	sdl=-ISDL/windows_mingw/include/SDL -LSDL/windows_mingw/lib -lSDLmain -lSDL
	chipmunk=-Ichipmunk/include -Lchipmunk/windows -lChipmunk
else ifeq ($(SYS),Darwin) #mac
	suffix=
	postcode = rm -rf bin/space.app/ && \
		cp -R resources/mac/space.app/ bin/space.app && \
		cp bin/main bin/space.app/Contents/MacOS && \
		install_name_tool -change /opt/local/lib/libSDL-1.2.0.dylib @executable_path/libSDL.dylib bin/space.app/Contents/MacOS/main
	os_lib=-framework cocoa
	openGL=-framework OpenGL
	sdl=-ISDL/mac -LSDL/mac -lSDLmain -lSDL
	chipmunk=-Ichipmunk/include -Lchipmunk/mac -lChipmunk
endif

main : src/main.c
	$(precode)
	gcc -o bin/main$(suffix) $(gamefiles) $(os_lib) $(openGL) $(sdl) $(chipmunk)
	$(postcode)
	
#mingw and linux
#gcc src/*.c src/constraints/*.c -Iinclude/chipmunk -o libChipmunk.a -std=c99 -lm -shared -fPIC
