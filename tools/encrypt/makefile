SUFFIX=

ifeq ($(SYS),Linux) #linux
	os=1
	SUFFIX=
else ifneq ($(findstring MINGW32_NT, $(SYS)),) #windows
	os=2
	SUFFIX=.exe
else ifeq ($(SYS),Darwin) #mac
	os=3
	SUFFIX=
endif

all : encrypt
clean :
	rm encrypt

encrypt : encrypt.c we_crypt.c
	gcc -o encrypt$(SUFFIX) $^
