
#ifndef SPACEENGINE_H_
#define SPACEENGINE_H_



cpShape *se_add_circle_shape(cpBody *body, cpFloat radius, cpFloat friction, cpFloat elasticity);
cpShape *se_add_box_shape(cpBody *body, cpFloat width, cpFloat height, cpFloat friction, cpFloat elasticity);
cpFloat se_get_best_shoot_angle(cpBody *a, cpBody *b,cpFloat bullet_speed);
cpFloat get_angle(cpBody *a, cpBody *b);
void se_add_score_and_popup(cpVect p, int score);
void se_add_explotion_at_contact_point(cpArbiter *arb);


#endif /* SPACEENGINE_H_ */
