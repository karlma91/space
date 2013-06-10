NAME = textures
COMPRESS_LEVEL = 9

all : ../.temporary_res/$(NAME).zip

../.temporary_res/$(NAME).zip : ./**
	rm ../.temporary_res/$(NAME).zip;
	zip -rX$(COMPRESS_LEVEL) ../.temporary_res/$(NAME).zip ./**.png -x "*/.*" -x ".*" -x "makefile"
