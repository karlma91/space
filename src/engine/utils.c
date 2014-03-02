#include "we_utils.h"

void strtolower(char *to, const char *from)
{
	int i;
	for (i=0; from[i]; i++) {
		to[i] = tolower(from[i]);
	}
	to[i] = '\0';
}

int EndsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}


Color color_new3b(byte r, byte g, byte b)
{
	Color col = {r,g,b,255};
	return col;
}

Color color_new4b(byte r, byte g, byte b, byte a)
{
	Color col = {r,g,b,a};
	return col;
}

Color color_new3f(float r, float g, float b)
{
	Color col = {(byte)(r*0xFF), (byte)(g*0xFF), (byte)(b*0xFF), 255};
	return col;
}

Color color_new4f(float r, float g, float b, float a)
{
	Color col = {(byte)(r*0xFF), (byte)(g*0xFF), (byte)(b*0xFF), (byte)(a*0xFF)};
	return col;
}

