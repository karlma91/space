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

all : stats
clean :
	rm stats

stats : stats.c
	gcc -o stats$(SUFFIX) $^
