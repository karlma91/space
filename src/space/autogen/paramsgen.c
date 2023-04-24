/* GENERATED FILE DO NOT CHANGE */

#include "paramsgen.h"
#include "we.h"
void object_types_init(void)
{
  OBJECT_REGISTER(tank);
  OBJECT_REGISTER(rocket);
  OBJECT_REGISTER(bullet);
  OBJECT_REGISTER(turret);
  OBJECT_REGISTER(spiky);
  OBJECT_REGISTER(crate);
  OBJECT_REGISTER(factory);
  OBJECT_REGISTER(robotarm);
  OBJECT_REGISTER(player);
  OBJECT_REGISTER(spikeball);
  OBJECT_REGISTER(staticpolygon);
  OBJECT_REGISTER(explosion);
  OBJECT_REGISTER(coin);
}
void* parse_generated(cJSON *param, char* type, char *name) 
{
void *ptr;  object_id *obj_id = object_by_name(type);
  union {
      obj_param_tank tank;
      obj_param_rocket rocket;
      obj_param_bullet bullet;
      obj_param_turret turret;
      obj_param_spiky spiky;
      obj_param_crate crate;
      obj_param_factory factory;
      obj_param_robotarm robotarm;
      obj_param_player player;
      obj_param_spikeball spikeball;
      obj_param_staticpolygon staticpolygon;
      obj_param_explosion explosion;
      obj_param_coin coin;
  } arg;
  strcpy((char*) &arg, name);
  if (obj_id == obj_id_tank) {
      obj_param_tank *temp = param_get("tank", "def");
      jparse_parse(param,"shoot_vel", "float" ,&(arg.tank.shoot_vel),(((ptr = NULL) || temp) && (ptr = &(temp->shoot_vel)), ptr));
      jparse_parse(param,"bullet_param", "char" ,&(arg.tank.bullet_param),(((ptr = NULL) || temp) && (ptr = &(temp->bullet_param)), ptr));
      jparse_parse(param,"mass_wheel", "float" ,&(arg.tank.mass_wheel),(((ptr = NULL) || temp) && (ptr = &(temp->mass_wheel)), ptr));
      jparse_parse(param,"coins", "int" ,&(arg.tank.coins),(((ptr = NULL) || temp) && (ptr = &(temp->coins)), ptr));
      jparse_parse(param,"bullet_type", "object_id" ,&(arg.tank.bullet_type),(((ptr = NULL) || temp) && (ptr = &(temp->bullet_type)), ptr));
      jparse_parse(param,"mass_body", "float" ,&(arg.tank.mass_body),(((ptr = NULL) || temp) && (ptr = &(temp->mass_body)), ptr));
      jparse_parse(param,"max_hp", "int" ,&(arg.tank.max_hp),(((ptr = NULL) || temp) && (ptr = &(temp->max_hp)), ptr));
      jparse_parse(param,"mass_barrel", "float" ,&(arg.tank.mass_barrel),(((ptr = NULL) || temp) && (ptr = &(temp->mass_barrel)), ptr));
      jparse_parse(param,"expl_pname", "char" ,&(arg.tank.expl_pname),(((ptr = NULL) || temp) && (ptr = &(temp->expl_pname)), ptr));
  } else if (obj_id == obj_id_rocket) {
      obj_param_rocket *temp = param_get("rocket", "def");
      jparse_parse(param,"expl_pname", "char" ,&(arg.rocket.expl_pname),(((ptr = NULL) || temp) && (ptr = &(temp->expl_pname)), ptr));
      jparse_parse(param,"coins", "int" ,&(arg.rocket.coins),(((ptr = NULL) || temp) && (ptr = &(temp->coins)), ptr));
      jparse_parse(param,"force", "float" ,&(arg.rocket.force),(((ptr = NULL) || temp) && (ptr = &(temp->force)), ptr));
      jparse_parse(param,"damage", "float" ,&(arg.rocket.damage),(((ptr = NULL) || temp) && (ptr = &(temp->damage)), ptr));
      jparse_parse(param,"max_hp", "float" ,&(arg.rocket.max_hp),(((ptr = NULL) || temp) && (ptr = &(temp->max_hp)), ptr));
  } else if (obj_id == obj_id_bullet) {
      obj_param_bullet *temp = param_get("bullet", "def");
      jparse_parse(param,"alive_time", "float" ,&(arg.bullet.alive_time),(((ptr = NULL) || temp) && (ptr = &(temp->alive_time)), ptr));
      jparse_parse(param,"expl_pname", "char" ,&(arg.bullet.expl_pname),(((ptr = NULL) || temp) && (ptr = &(temp->expl_pname)), ptr));
      jparse_parse(param,"spr_id", "sprite" ,&(arg.bullet.spr_id),(((ptr = NULL) || temp) && (ptr = &(temp->spr_id)), ptr));
      jparse_parse(param,"fade_time", "float" ,&(arg.bullet.fade_time),(((ptr = NULL) || temp) && (ptr = &(temp->fade_time)), ptr));
      jparse_parse(param,"col", "Color" ,&(arg.bullet.col),(((ptr = NULL) || temp) && (ptr = &(temp->col)), ptr));
      jparse_parse(param,"radius", "float" ,&(arg.bullet.radius),(((ptr = NULL) || temp) && (ptr = &(temp->radius)), ptr));
      jparse_parse(param,"mass", "float" ,&(arg.bullet.mass),(((ptr = NULL) || temp) && (ptr = &(temp->mass)), ptr));
      jparse_parse(param,"render_size", "float" ,&(arg.bullet.render_size),(((ptr = NULL) || temp) && (ptr = &(temp->render_size)), ptr));
      jparse_parse(param,"render_stretch", "int" ,&(arg.bullet.render_stretch),(((ptr = NULL) || temp) && (ptr = &(temp->render_stretch)), ptr));
      jparse_parse(param,"damage", "float" ,&(arg.bullet.damage),(((ptr = NULL) || temp) && (ptr = &(temp->damage)), ptr));
      jparse_parse(param,"friendly", "int" ,&(arg.bullet.friendly),(((ptr = NULL) || temp) && (ptr = &(temp->friendly)), ptr));
  } else if (obj_id == obj_id_turret) {
      obj_param_turret *temp = param_get("turret", "def");
      jparse_parse(param,"shoot_vel", "float" ,&(arg.turret.shoot_vel),(((ptr = NULL) || temp) && (ptr = &(temp->shoot_vel)), ptr));
      jparse_parse(param,"bullet_param", "char" ,&(arg.turret.bullet_param),(((ptr = NULL) || temp) && (ptr = &(temp->bullet_param)), ptr));
      jparse_parse(param,"expl_pname", "char" ,&(arg.turret.expl_pname),(((ptr = NULL) || temp) && (ptr = &(temp->expl_pname)), ptr));
      jparse_parse(param,"coins", "int" ,&(arg.turret.coins),(((ptr = NULL) || temp) && (ptr = &(temp->coins)), ptr));
      jparse_parse(param,"bullet_type", "object_id" ,&(arg.turret.bullet_type),(((ptr = NULL) || temp) && (ptr = &(temp->bullet_type)), ptr));
      jparse_parse(param,"turret_size", "float" ,&(arg.turret.turret_size),(((ptr = NULL) || temp) && (ptr = &(temp->turret_size)), ptr));
      jparse_parse(param,"burst_number", "int" ,&(arg.turret.burst_number),(((ptr = NULL) || temp) && (ptr = &(temp->burst_number)), ptr));
      jparse_parse(param,"mass", "float" ,&(arg.turret.mass),(((ptr = NULL) || temp) && (ptr = &(temp->mass)), ptr));
      jparse_parse(param,"max_hp", "int" ,&(arg.turret.max_hp),(((ptr = NULL) || temp) && (ptr = &(temp->max_hp)), ptr));
      jparse_parse(param,"shape_id", "shape" ,&(arg.turret.shape_id),(((ptr = NULL) || temp) && (ptr = &(temp->shape_id)), ptr));
      jparse_parse(param,"rot_speed", "float" ,&(arg.turret.rot_speed),(((ptr = NULL) || temp) && (ptr = &(temp->rot_speed)), ptr));
      jparse_parse(param,"shoot_interval", "float" ,&(arg.turret.shoot_interval),(((ptr = NULL) || temp) && (ptr = &(temp->shoot_interval)), ptr));
  } else if (obj_id == obj_id_spiky) {
      obj_param_spiky *temp = param_get("spiky", "def");
  } else if (obj_id == obj_id_crate) {
      obj_param_crate *temp = param_get("crate", "def");
      jparse_parse(param,"sprite", "sprite" ,&(arg.crate.sprite),(((ptr = NULL) || temp) && (ptr = &(temp->sprite)), ptr));
      jparse_parse(param,"coins", "int" ,&(arg.crate.coins),(((ptr = NULL) || temp) && (ptr = &(temp->coins)), ptr));
      jparse_parse(param,"invinc", "int" ,&(arg.crate.invinc),(((ptr = NULL) || temp) && (ptr = &(temp->invinc)), ptr));
      jparse_parse(param,"max_hp", "int" ,&(arg.crate.max_hp),(((ptr = NULL) || temp) && (ptr = &(temp->max_hp)), ptr));
      jparse_parse(param,"expl_pname", "char" ,&(arg.crate.expl_pname),(((ptr = NULL) || temp) && (ptr = &(temp->expl_pname)), ptr));
      jparse_parse(param,"size", "float" ,&(arg.crate.size),(((ptr = NULL) || temp) && (ptr = &(temp->size)), ptr));
  } else if (obj_id == obj_id_factory) {
      obj_param_factory *temp = param_get("factory", "def");
      jparse_parse(param,"param_name", "char" ,&(arg.factory.param_name),(((ptr = NULL) || temp) && (ptr = &(temp->param_name)), ptr));
      jparse_parse(param,"spawn_type", "object_id" ,&(arg.factory.spawn_type),(((ptr = NULL) || temp) && (ptr = &(temp->spawn_type)), ptr));
      jparse_parse(param,"sprite", "sprite" ,&(arg.factory.sprite),(((ptr = NULL) || temp) && (ptr = &(temp->sprite)), ptr));
      jparse_parse(param,"coins", "int" ,&(arg.factory.coins),(((ptr = NULL) || temp) && (ptr = &(temp->coins)), ptr));
      jparse_parse(param,"spawn_delay", "float" ,&(arg.factory.spawn_delay),(((ptr = NULL) || temp) && (ptr = &(temp->spawn_delay)), ptr));
      jparse_parse(param,"expl_pname", "char" ,&(arg.factory.expl_pname),(((ptr = NULL) || temp) && (ptr = &(temp->expl_pname)), ptr));
      jparse_parse(param,"shape", "shape" ,&(arg.factory.shape),(((ptr = NULL) || temp) && (ptr = &(temp->shape)), ptr));
      jparse_parse(param,"max_hp", "float" ,&(arg.factory.max_hp),(((ptr = NULL) || temp) && (ptr = &(temp->max_hp)), ptr));
      jparse_parse(param,"max_tanks", "int" ,&(arg.factory.max_tanks),(((ptr = NULL) || temp) && (ptr = &(temp->max_tanks)), ptr));
  } else if (obj_id == obj_id_robotarm) {
      obj_param_robotarm *temp = param_get("robotarm", "def");
      jparse_parse(param,"force", "float" ,&(arg.robotarm.force),(((ptr = NULL) || temp) && (ptr = &(temp->force)), ptr));
      jparse_parse(param,"max_vel", "float" ,&(arg.robotarm.max_vel),(((ptr = NULL) || temp) && (ptr = &(temp->max_vel)), ptr));
      jparse_parse(param,"seg_length", "int" ,&(arg.robotarm.seg_length),(((ptr = NULL) || temp) && (ptr = &(temp->seg_length)), ptr));
      jparse_parse(param,"segments", "int" ,&(arg.robotarm.segments),(((ptr = NULL) || temp) && (ptr = &(temp->segments)), ptr));
      jparse_parse(param,"coins", "int" ,&(arg.robotarm.coins),(((ptr = NULL) || temp) && (ptr = &(temp->coins)), ptr));
      jparse_parse(param,"max_hp", "int" ,&(arg.robotarm.max_hp),(((ptr = NULL) || temp) && (ptr = &(temp->max_hp)), ptr));
      jparse_parse(param,"expl_pname", "char" ,&(arg.robotarm.expl_pname),(((ptr = NULL) || temp) && (ptr = &(temp->expl_pname)), ptr));
  } else if (obj_id == obj_id_player) {
      obj_param_player *temp = param_get("player", "def");
      jparse_parse(param,"player_id", "int" ,&(arg.player.player_id),(((ptr = NULL) || temp) && (ptr = &(temp->player_id)), ptr));
  } else if (obj_id == obj_id_spikeball) {
      obj_param_spikeball *temp = param_get("spikeball", "def");
      jparse_parse(param,"top_delay", "float" ,&(arg.spikeball.top_delay),(((ptr = NULL) || temp) && (ptr = &(temp->top_delay)), ptr));
      jparse_parse(param,"bot_delay", "float" ,&(arg.spikeball.bot_delay),(((ptr = NULL) || temp) && (ptr = &(temp->bot_delay)), ptr));
      jparse_parse(param,"up_speed", "float" ,&(arg.spikeball.up_speed),(((ptr = NULL) || temp) && (ptr = &(temp->up_speed)), ptr));
      jparse_parse(param,"damage", "float" ,&(arg.spikeball.damage),(((ptr = NULL) || temp) && (ptr = &(temp->damage)), ptr));
      jparse_parse(param,"down_force", "float" ,&(arg.spikeball.down_force),(((ptr = NULL) || temp) && (ptr = &(temp->down_force)), ptr));
      jparse_parse(param,"radius", "float" ,&(arg.spikeball.radius),(((ptr = NULL) || temp) && (ptr = &(temp->radius)), ptr));
  } else if (obj_id == obj_id_staticpolygon) {
      obj_param_staticpolygon *temp = param_get("staticpolygon", "def");
      jparse_parse(param,"texture_scale", "float" ,&(arg.staticpolygon.texture_scale),(((ptr = NULL) || temp) && (ptr = &(temp->texture_scale)), ptr));
      jparse_parse(param,"shape_id", "shape" ,&(arg.staticpolygon.shape_id),(((ptr = NULL) || temp) && (ptr = &(temp->shape_id)), ptr));
      jparse_parse(param,"tex_name", "char" ,&(arg.staticpolygon.tex_name),(((ptr = NULL) || temp) && (ptr = &(temp->tex_name)), ptr));
      jparse_parse(param,"outline", "int" ,&(arg.staticpolygon.outline),(((ptr = NULL) || temp) && (ptr = &(temp->outline)), ptr));
      jparse_parse(param,"scale", "float" ,&(arg.staticpolygon.scale),(((ptr = NULL) || temp) && (ptr = &(temp->scale)), ptr));
  } else if (obj_id == obj_id_explosion) {
      obj_param_explosion *temp = param_get("explosion", "def");
      jparse_parse(param,"snd", "sound" ,&(arg.explosion.snd),(((ptr = NULL) || temp) && (ptr = &(temp->snd)), ptr));
      jparse_parse(param,"force", "float" ,&(arg.explosion.force),(((ptr = NULL) || temp) && (ptr = &(temp->force)), ptr));
      jparse_parse(param,"dmg", "float" ,&(arg.explosion.dmg),(((ptr = NULL) || temp) && (ptr = &(temp->dmg)), ptr));
      jparse_parse(param,"seconds", "float" ,&(arg.explosion.seconds),(((ptr = NULL) || temp) && (ptr = &(temp->seconds)), ptr));
      jparse_parse(param,"em_expl", "emitter" ,&(arg.explosion.em_expl),(((ptr = NULL) || temp) && (ptr = &(temp->em_expl)), ptr));
      jparse_parse(param,"em_frag", "emitter" ,&(arg.explosion.em_frag),(((ptr = NULL) || temp) && (ptr = &(temp->em_frag)), ptr));
      jparse_parse(param,"size", "float" ,&(arg.explosion.size),(((ptr = NULL) || temp) && (ptr = &(temp->size)), ptr));
  } else if (obj_id == obj_id_coin) {
      obj_param_coin *temp = param_get("coin", "def");
      jparse_parse(param,"explo_fmax", "float" ,&(arg.coin.explo_fmax),(((ptr = NULL) || temp) && (ptr = &(temp->explo_fmax)), ptr));
  }
  const int paramsize = obj_id->P_SIZE;
  void * data = calloc(1, paramsize);
  memcpy(data, &arg, paramsize);
  return data;
}
cJSON * write_generated(object_id *obj_id, void *data,  char *type, char *name) 
{
  cJSON *param = cJSON_CreateObject();
  cJSON_AddItemToObject(param, "type", cJSON_CreateString(type));
  cJSON_AddItemToObject(param, "name", cJSON_CreateString(name));
  if (obj_id == obj_id_tank) {
    obj_param_tank * tank = (obj_param_tank*) data;
    cJSON_AddNumberToObject(param,"shoot_vel",tank->shoot_vel);
    cJSON_AddItemToObject(param,"bullet_param",cJSON_CreateString(tank->bullet_param));
    cJSON_AddNumberToObject(param,"mass_wheel",tank->mass_wheel);
    cJSON_AddNumberToObject(param,"coins",tank->coins);
    cJSON_AddItemToObject(param,"bullet_type",cJSON_CreateString(tank->bullet_type->NAME));
    cJSON_AddNumberToObject(param,"mass_body",tank->mass_body);
    cJSON_AddNumberToObject(param,"max_hp",tank->max_hp);
    cJSON_AddNumberToObject(param,"mass_barrel",tank->mass_barrel);
    cJSON_AddItemToObject(param,"expl_pname",cJSON_CreateString(tank->expl_pname));
  } else if (obj_id == obj_id_rocket) {
    obj_param_rocket * rocket = (obj_param_rocket*) data;
    cJSON_AddItemToObject(param,"expl_pname",cJSON_CreateString(rocket->expl_pname));
    cJSON_AddNumberToObject(param,"coins",rocket->coins);
    cJSON_AddNumberToObject(param,"force",rocket->force);
    cJSON_AddNumberToObject(param,"damage",rocket->damage);
    cJSON_AddNumberToObject(param,"max_hp",rocket->max_hp);
  } else if (obj_id == obj_id_bullet) {
    obj_param_bullet * bullet = (obj_param_bullet*) data;
    cJSON_AddNumberToObject(param,"alive_time",bullet->alive_time);
    cJSON_AddItemToObject(param,"expl_pname",cJSON_CreateString(bullet->expl_pname));
    cJSON_AddItemToObject(param,"spr_id",cJSON_CreateString(sprite_get_name(bullet->spr_id)));
    cJSON_AddNumberToObject(param,"fade_time",bullet->fade_time);
    cJSON_AddNumberToObject(param,"radius",bullet->radius);
    cJSON_AddNumberToObject(param,"mass",bullet->mass);
    cJSON_AddNumberToObject(param,"render_size",bullet->render_size);
    cJSON_AddNumberToObject(param,"render_stretch",bullet->render_stretch);
    cJSON_AddNumberToObject(param,"damage",bullet->damage);
    cJSON_AddNumberToObject(param,"friendly",bullet->friendly);
  } else if (obj_id == obj_id_turret) {
    obj_param_turret * turret = (obj_param_turret*) data;
    cJSON_AddNumberToObject(param,"shoot_vel",turret->shoot_vel);
    cJSON_AddItemToObject(param,"bullet_param",cJSON_CreateString(turret->bullet_param));
    cJSON_AddItemToObject(param,"expl_pname",cJSON_CreateString(turret->expl_pname));
    cJSON_AddNumberToObject(param,"coins",turret->coins);
    cJSON_AddItemToObject(param,"bullet_type",cJSON_CreateString(turret->bullet_type->NAME));
    cJSON_AddNumberToObject(param,"turret_size",turret->turret_size);
    cJSON_AddNumberToObject(param,"burst_number",turret->burst_number);
    cJSON_AddNumberToObject(param,"mass",turret->mass);
    cJSON_AddNumberToObject(param,"max_hp",turret->max_hp);
    cJSON_AddItemToObject(param,"shape_id",cJSON_CreateString(turret->shape_id->name));
    cJSON_AddNumberToObject(param,"rot_speed",turret->rot_speed);
    cJSON_AddNumberToObject(param,"shoot_interval",turret->shoot_interval);
  } else if (obj_id == obj_id_spiky) {
    obj_param_spiky * spiky = (obj_param_spiky*) data;
  } else if (obj_id == obj_id_crate) {
    obj_param_crate * crate = (obj_param_crate*) data;
    cJSON_AddItemToObject(param,"sprite",cJSON_CreateString(sprite_get_name(crate->sprite)));
    cJSON_AddNumberToObject(param,"coins",crate->coins);
    cJSON_AddNumberToObject(param,"invinc",crate->invinc);
    cJSON_AddNumberToObject(param,"max_hp",crate->max_hp);
    cJSON_AddItemToObject(param,"expl_pname",cJSON_CreateString(crate->expl_pname));
    cJSON_AddNumberToObject(param,"size",crate->size);
  } else if (obj_id == obj_id_factory) {
    obj_param_factory * factory = (obj_param_factory*) data;
    cJSON_AddItemToObject(param,"param_name",cJSON_CreateString(factory->param_name));
    cJSON_AddItemToObject(param,"spawn_type",cJSON_CreateString(factory->spawn_type->NAME));
    cJSON_AddItemToObject(param,"sprite",cJSON_CreateString(sprite_get_name(factory->sprite)));
    cJSON_AddNumberToObject(param,"coins",factory->coins);
    cJSON_AddNumberToObject(param,"spawn_delay",factory->spawn_delay);
    cJSON_AddItemToObject(param,"expl_pname",cJSON_CreateString(factory->expl_pname));
    cJSON_AddItemToObject(param,"shape",cJSON_CreateString(factory->shape->name));
    cJSON_AddNumberToObject(param,"max_hp",factory->max_hp);
    cJSON_AddNumberToObject(param,"max_tanks",factory->max_tanks);
  } else if (obj_id == obj_id_robotarm) {
    obj_param_robotarm * robotarm = (obj_param_robotarm*) data;
    cJSON_AddNumberToObject(param,"force",robotarm->force);
    cJSON_AddNumberToObject(param,"max_vel",robotarm->max_vel);
    cJSON_AddNumberToObject(param,"seg_length",robotarm->seg_length);
    cJSON_AddNumberToObject(param,"segments",robotarm->segments);
    cJSON_AddNumberToObject(param,"coins",robotarm->coins);
    cJSON_AddNumberToObject(param,"max_hp",robotarm->max_hp);
    cJSON_AddItemToObject(param,"expl_pname",cJSON_CreateString(robotarm->expl_pname));
  } else if (obj_id == obj_id_player) {
    obj_param_player * player = (obj_param_player*) data;
    cJSON_AddNumberToObject(param,"player_id",player->player_id);
  } else if (obj_id == obj_id_spikeball) {
    obj_param_spikeball * spikeball = (obj_param_spikeball*) data;
    cJSON_AddNumberToObject(param,"top_delay",spikeball->top_delay);
    cJSON_AddNumberToObject(param,"bot_delay",spikeball->bot_delay);
    cJSON_AddNumberToObject(param,"up_speed",spikeball->up_speed);
    cJSON_AddNumberToObject(param,"damage",spikeball->damage);
    cJSON_AddNumberToObject(param,"down_force",spikeball->down_force);
    cJSON_AddNumberToObject(param,"radius",spikeball->radius);
  } else if (obj_id == obj_id_staticpolygon) {
    obj_param_staticpolygon * staticpolygon = (obj_param_staticpolygon*) data;
    cJSON_AddNumberToObject(param,"texture_scale",staticpolygon->texture_scale);
    cJSON_AddItemToObject(param,"shape_id",cJSON_CreateString(staticpolygon->shape_id->name));
    cJSON_AddItemToObject(param,"tex_name",cJSON_CreateString(staticpolygon->tex_name));
    cJSON_AddNumberToObject(param,"outline",staticpolygon->outline);
    cJSON_AddNumberToObject(param,"scale",staticpolygon->scale);
  } else if (obj_id == obj_id_explosion) {
    obj_param_explosion * explosion = (obj_param_explosion*) data;
    cJSON_AddItemToObject(param,"snd",cJSON_CreateString(sound_get_name(explosion->snd)));
    cJSON_AddNumberToObject(param,"force",explosion->force);
    cJSON_AddNumberToObject(param,"dmg",explosion->dmg);
    cJSON_AddNumberToObject(param,"seconds",explosion->seconds);
    cJSON_AddItemToObject(param,"em_expl",cJSON_CreateString(particles_get_name(explosion->em_expl)));
    cJSON_AddItemToObject(param,"em_frag",cJSON_CreateString(particles_get_name(explosion->em_frag)));
    cJSON_AddNumberToObject(param,"size",explosion->size);
  } else if (obj_id == obj_id_coin) {
    obj_param_coin * coin = (obj_param_coin*) data;
    cJSON_AddNumberToObject(param,"explo_fmax",coin->explo_fmax);
  }
  return param;
}
