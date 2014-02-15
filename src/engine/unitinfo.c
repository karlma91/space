/*
 * unitinfo.c
 *
 *  Created on: 6. jan. 2014
 *      Author: Mathias
 */

#include <stdlib.h>
#include <stdio.h>
#include "we_unitinfo.h"
#if !__WIN32__
#include <sys/utsname.h>
struct utsname unameData;
#endif

we_unitinfo we_info;

#include "../space/game.h"

void we_info_init(void)
{
#if !TARGET_OS_IPHONE && !__WIN32__
    char *user = getenv("USER");
    if (user)
    	strncpy(we_info.name, user, 50);
    else
    	fprintf(stderr, "USER not found\n");

    if (uname(&unameData)) {
    	fprintf(stderr, "warning: could not get system info\n");
    } else {
    	strncpy(we_info.systemName, unameData.sysname, 50);
    	strncpy(we_info.systemVersion, unameData.release, 50);
    	strncpy(we_info.model, unameData.machine, 50);
    	strncpy(we_info.modelName, unameData.nodename, 50);
    }
#endif

	we_info_print();
}

void we_info_print(void)
{
	fprintf(stderr, "UNIT INFO\n");
	fprintf(stderr, "NAME: %s\n", we_info.name);
	fprintf(stderr, "SYSTEM: %s\n", we_info.systemName);
	fprintf(stderr, "VERSION: %s\n", we_info.systemVersion);
	fprintf(stderr, "MODEL: %s\n", we_info.model);
	fprintf(stderr, "MODEL NAME: %s\n\n", we_info.modelName);
}

void we_info_coord_update(void)
{
	fprintf(stderr, "COORDINATES: latitude: %f longitude: %f\n", we_info.location[0], we_info.location[1]);
}
