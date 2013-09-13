/*
 * tween.c
 *
 *  Created on: Sep 13, 2013
 *      Author: karlmka
 */
#include "tween.h"
#include "easing.h"

float tween_move_f(float start, float end, float time, float (*easing)(float))
{
	return start + easing(time) * (end - start);
}
