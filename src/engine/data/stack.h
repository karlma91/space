/*
 * stack.h
 *
 *  Created on: 14. juni 2013
 *      Author: Karl
 */

#ifndef STACK_H_
#define STACK_H_

#include "we_utils.h"

int stack_pop_int(void);
byte stack_pop_byte(void);
void stack_push_int(int a);
void stack_push_byte(byte a);

#endif /* STACK_H_ */
