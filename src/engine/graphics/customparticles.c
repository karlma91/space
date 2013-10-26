#include "particles.h"
#include "customparticles.h"
#include "draw.h"
#include <stdio.h>
#include "../../space/game.h"

static void draw_particle_as_score(emitter *em, particle *p);
static void draw_particle_as_spark(emitter *em, particle *p);

emitter *particles_add_score_popup(particle_system *s, int layer, cpVect p,int score)
{
    emitter *e = particles_get_emitter_at(s, layer, EMITTER_SCORE, p);
    if(e){
        e->data = (void*)score;
        e->draw_particle = draw_particle_as_score;
        return e;
    }else{
        return NULL;
    }
}

emitter *particles_add_sparks(particle_system *s, int layer, cpVect p, float angle, float force)
{
    emitter *e = particles_get_emitter_at(s, RLAY_GAME_FRONT, EMITTER_SPARKS, p);
    if(e){
        float intensity = 0;
        if(force > 1000){
            intensity = 1;
        }else if(force < 250){
            intensity = 0.25;
        }else{
            intensity = force/2000;
        }
        e->draw_particle = draw_particle_as_spark;
        e->angular_offset = angle + WE_PI_2;
        e->speed.min = 0;
        e->speed.max = intensity*3000;
        e->init_life.min = 0;
        e->init_life.max = intensity*120;
        return e;
    }else{
        return NULL;
    }
}

static void draw_particle_as_score(emitter *em, particle *p)
{
    char temp[20];
    int score = ((int)em->data);
    sprintf(temp,"%d",score);
    setTextAlign(TEXT_CENTER);
    setTextSize(p->size);

    float angle = 0;
    cpVect pos = p->p;

    setTextAngle(angle);
    font_drawText(em->layer, pos.x,pos.y,temp);
    setTextAngle(0);
}


static void draw_particle_as_spark(emitter *em, particle *p)
{
    cpVect pos = p->p;
    draw_glow_line(pos,cpvadd(pos, cpvmult(p->v, p->size)), p->size);
}
