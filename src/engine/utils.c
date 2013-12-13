#include "we_utils.h"

void strtolower(char *to, const char *from)
{
	int i;
	for (i=0; from[i]; i++) {
		to[i] = tolower(from[i]);
	}
	to[i] = '\0';
}
