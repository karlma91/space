#include "particles.h"
#include "customparticles.h"
#include "draw.h"
#include <stdio.h>
#include "../../space/game.h"

static void draw_particle_as_score(emitter *em, particle *p);
static void draw_particle_as_spark(emitter *em, particle *p);

emitter *particles_add_score_popup(cpVect p,int score)
{
    emitter *e = particles_get_emitter_at(EMITTER_SCORE, p);
    if(e){
        e->data = (void*)score;
        e->draw_particle = draw_particle_as_score;
        return e;
    }else{
        return NULL;
    }
}

emitter *particles_add_sparks(cpVect p, float angle, float force)
{
    emitter *e = particles_get_emitter_at(EMITTER_SPARKS, p);
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
        e->angular_offset = angle * (180/M_PI)+90;
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
#if EXPERIMENTAL_GRAPHICS
    //angle = se_rect2arch(&pos) * 180 / M_PI;
#endif

    setTextAngle(angle);
    font_drawText(pos.x,pos.y,temp);
    setTextAngle(0);
}


static void draw_particle_as_spark(emitter *em, particle *p)
{
    cpVect pos = p->p;
#if EXPERIMENTAL_GRAPHICS
    se_rect2arch(&pos);
#endif
    draw_glow_line(pos.x,pos.y,pos.x + p->v.x*p->size,pos.y+ p->v.y*p->size, p->size);
}
