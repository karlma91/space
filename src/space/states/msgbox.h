/*
 * textinput.h
 *
 *  Created on: Jan 18, 2014
 *      Author: karlmka
 */

#ifndef MSGBOX_H_
#define MSGBOX_H_
#include "we.h"

#define MSGBOX_OK 1
#define MSGBOX_CANCEL 2
typedef void (*msg_call_t)(int);

void msgbox_show(const char * message, msg_call_t msg_call);

#endif /* MSGBOX_H_ */
