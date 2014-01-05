SUFFIX=

all : encrypt
clean :
	rm encrypt

encrypt : encrypt.c we_crypt.c
	gcc -o encrypt$(SUFFIX) $^
