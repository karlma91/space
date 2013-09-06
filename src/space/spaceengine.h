#ifndef SPACEENGINE_H_
#define SPACEENGINE_H_
#include "chipmunk.h"
#include "../engine/components/object.h"

cpFloat se_get_best_shoot_angle(cpVect a, cpVect va, cpVect b, cpVect vb, cpFloat bullet_speed);
cpFloat get_angle(cpVect a, cpVect b);
void se_add_score_and_popup(cpVect p, int score);
int se_damage_object(instance *object, instance *dmg_dealer);
float turn_toangle(float from_angle, float to_angle, float step_size);

float se_distance_to_player(float x);
cpVect se_distance_a2b(instance *insa, instance *insb);
cpVect se_distance_v(cpVect a, cpVect b);

float se_rect2arch(cpVect *pos);
void se_rect2arch_column(float x, cpVect *polar);
void se_rect2arch_from_data(cpVect *pos, cpVect *polar);

int se_inside_view(cpVect *pos, float margin);
void se_spawn_coins(instance *ins);

#endif /* SPACEENGINE_H_ */
