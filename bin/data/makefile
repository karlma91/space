NAME = data
COMPRESS_LEVEL = 9

all : ../../.temporary_res/data.zip

../../.temporary_res/data.zip : ./** ./**/**
	rm -f ../../.temporary_res/$(NAME).zip;
	zip -rX$(COMPRESS_LEVEL) ../../.temporary_res/$(NAME).zip ./ -x "*/.*" -x ".*" -x "makefile"
