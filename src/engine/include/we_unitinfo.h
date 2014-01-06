/*
 * we_info.h
 *
 *  Created on: 6. jan. 2014
 *      Author: Mathias
 */

#ifndef WE_INFO_H_
#define WE_INFO_H_

typedef struct we_unitinfo {
	char name[50];
	char model[50];
	char modelName[50];
	char systemName[50];
	char systemVersion[50];
	double location[2];
} we_unitinfo;

extern we_unitinfo we_info;

void we_info_init(void);
void we_info_print(void);
void we_info_coord_update(void);

#endif /* WE_INFO_H_ */
