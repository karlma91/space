SYS := $(firstword $(shell uname -s))

ifeq ($(SYS),Linux) #linux
os=linux
suffix=.a
else ifneq ($(findstring MINGW32_NT, $(SYS)),) #windows
os=win
suffix=.a
else ifeq ($(SYS),Darwin) #mac
os=mac
suffix=.a
endif

all: static-lib

static-lib: $(os)
	gcc -c src/ini.c -Iinclude -o ini.o
	ar rcs $(os)/libinih$(suffix) ini.o
	rm ini.o

$(os) :
	mkdir $(os)