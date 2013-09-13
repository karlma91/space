/*
 * tween.h
 *
 *  Created on: Sep 13, 2013
 *      Author: karlmka
 */

#ifndef TWEEN_H_
#define TWEEN_H_

float tween_move_f(float start, float end, float time, float (*easing)(float));

#endif /* TWEEN_H_ */
