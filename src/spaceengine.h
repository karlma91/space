#ifndef SPACEENGINE_H_
#define SPACEENGINE_H_
#include "chipmunk.h"
#include "objects.h"

cpShape *se_add_circle_shape(cpBody *body, cpFloat radius, cpFloat friction,
		cpFloat elasticity);
cpShape *se_add_box_shape(cpBody *body, cpFloat width, cpFloat height,
		cpFloat friction, cpFloat elasticity);
cpFloat se_get_best_shoot_angle(cpVect a, cpVect va, cpVect b, cpVect vb, cpFloat bullet_speed);
cpFloat get_angle(cpVect a, cpVect b);
void se_add_score_and_popup(cpVect p, int score);
int se_damage_object(object_data *object, float damage);
float turn_toangle(float from_angle, float to_angle, float step_size);
float se_distance_to_player(float x);
void se_shape_from_space(cpBody *body, cpShape *shape, void *data);
void se_constrain_from_space(cpBody *body, cpConstraint *constraint, void *data);

float se_rect2arch(cpVect *pos);
void se_rect2arch_column(float x, cpVect *polar);
void se_rect2arch_from_data(cpVect *pos, cpVect *polar);

int se_inside_view(cpVect *pos, float margin);

#endif /* SPACEENGINE_H_ */
