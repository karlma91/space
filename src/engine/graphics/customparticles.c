#include "particles.h"
#include "customparticles.h"
#include "draw.h"
#include <stdio.h>
#include "../../space/game.h"

static void draw_particle_as_score(emitter *em, particle *p);
static void draw_particle_as_spark(emitter *em, particle *p);
static void draw_particle_as_line(emitter *em, particle *p);

emitter *particles_add_score_popup(int layer, cpVect p,int score)
{
    emitter *e = particles_get_emitter_at(layer, EM_SCORE, p);
    if(e){
        e->data = (void*)score;
        e->draw_particle = draw_particle_as_score;
        return e;
    }else{
        return NULL;
    }
}

emitter *particles_add_sparks(int layer, cpVect p, float angle, float force)
{
    emitter *e = particles_get_emitter_at(RLAY_GAME_FRONT, EM_SPARKS, p);
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
        e->init_life.min = 50;
        e->init_life.max = intensity*300;
        return e;
    }else{
        return NULL;
    }
}

emitter *particles_add_color_expl(int layer, cpVect p, float force)
{
    emitter *e = particles_get_emitter_at(RLAY_GAME_FRONT, EM_SPARKS, p);
    if(e){
        e->draw_particle = draw_particle_as_line;
       // e->angular_offset = angle + WE_PI_2;
        e->spawn_count.min = 100;
        e->spawn_count.max = 200;
        e->spread = WE_2PI;
        e->speed.min = 200;
        e->speed.max = 300;
        e->init_life.min = 1000;
        e->init_life.max = 2000;
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

static void draw_particle_as_line(emitter *em, particle *p)
{
	draw_push_color();
	Color col = draw_get_current_color();
	col.a = 0;
	draw_color(col);
    cpVect pos = p->p;
    draw_color(COL_WHITE);
    cpVect b = cpvmult(p->v, 256);
    p->v = cpvmult(p->v, 0.99);
    cpVect d = cpvmult(b,0.5);
    pos = cpvsub(pos,d);
    draw_simple_line_spr_id(0, p->spr.id, 0, pos, cpvadd(pos, b), 7);
	draw_pop_color();
}

static void draw_particle_as_spark(emitter *em, particle *p)
{
    cpVect pos = p->p;
    draw_glow_line(pos,cpvadd(pos, cpvmult(p->v, p->size+16)), p->size);
}
