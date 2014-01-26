#ifndef SPACEENGINE_H_
#define SPACEENGINE_H_
#include "chipmunk.h"
#include "../engine/components/object.h"

cpFloat se_get_best_shoot_angle(cpBody *body1, cpBody *body2, cpFloat bullet_speed);
cpFloat get_angle(cpVect a, cpVect b);
void se_str_to_upper(char * str);
void se_add_score_and_popup(cpVect p, int score);
int se_damage_object(instance *object, instance *dmg_dealer);
int se_damage_deal(instance *object, float damage);
float turn_toangle(float from_angle, float to_angle, float step_size);

float se_arcdist2player(cpVect p);
cpVect se_dist_a2b(instance *insa, instance *insb);
cpVect se_dist2body(cpBody *b1, cpBody *b2);

cpVect se_vect_view2world(view *cam, cpVect p);

void se_spawn_coins(instance *ins);

cpFloat se_tangent(cpVect p);
void se_tangent_body(cpBody *body);
void se_velfunc(cpBody *body, int with_gravity);

#define SE_ALTITUDE(p) (currentlvl->outer_radius - cpvlength(p))

#endif /* SPACEENGINE_H_ */
