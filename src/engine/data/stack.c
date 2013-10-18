#include <assert.h>
#include "stack.h"

byte byte_stack[200];
int int_stack[50];

int byte_top = -1;
int int_top = -1;

int stack_pop_int(void)
{
	assert(int_top != -1);
	int a = int_stack[int_top];
	--int_top;
	return a;
}

void stack_push_int(int a)
{
	++int_top;
	assert(int_top < 50);
	int_stack[int_top] = a;
}


byte stack_pop_byte(void)
{
	assert(byte_top != -1);
	byte a = byte_stack[byte_top];
	--byte_top;
	return a;
}

void stack_push_byte(byte a)
{
	++byte_top;
	assert(byte_top < 50);
	byte_stack[byte_top] = a;
}

