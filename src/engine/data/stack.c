#include <assert.h>
int int_stack[50];
float float_stack[50];

int int_top = -1;
int float_top = -1;

void stack_push_float(float a)
{
	float_top += 1;
	float_stack[float_top] = a;
}

void stack_push_int(int a)
{
	int_top += 1;
	int_stack[int_top] = a;
}

float stack_pop_float(void)
{
	assert(float_top != -1);
	float a = float_stack[float_top];
	float_top -= 1;
	return a;
}

int stack_pop_int(void)
{
	assert(int_top != -1);
	int a = int_stack[int_top];
	int_top -= 1;
	return a;
}


