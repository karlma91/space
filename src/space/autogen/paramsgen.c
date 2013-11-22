/* GENERATED FILE DO NOT CHANGE */

#include "paramsgen.h"
void* parse_generated(cJSON *param, char* type, char *name) 
{
  object_id *obj_id = object_by_name(type);
  union {
      obj_param_staticpolygon staticpolygon;
      obj_param_player player;
      obj_param_bullet bullet;
      obj_param_tank tank;
      obj_param_factory factory;
      obj_param_turret turret;
      obj_param_rocket rocket;
      obj_param_robotarm robotarm;
      obj_param_spikeball spikeball;
      obj_param_coin coin;
  } arg;
  strcpy((char*) &arg, name);
cJSON * object_spawn = NULL;
  if (obj_id == obj_id_staticpolygon) {
    strcpy(arg.staticpolygon.spr_name, level_safe_parse_char(param, "spr_name"));
    arg.staticpolygon.outline = level_safe_parse_int(param, "outline");
    arg.staticpolygon.scale = level_safe_parse_float(param, "scale");
    arg.staticpolygon.texture_scale = level_safe_parse_float(param, "texture_scale");
    strcpy(arg.staticpolygon.shape_name, level_safe_parse_char(param, "shape_name"));
  } else if (obj_id == obj_id_player) {
    arg.player.max_hp = level_safe_parse_float(param, "max_hp");
    arg.player.tex_id = level_safe_parse_int(param, "tex_id");
    arg.player.player_id = level_safe_parse_int(param, "player_id");
    arg.player.gun_cooldown = level_safe_parse_float(param, "gun_cooldown");
    arg.player.cash_radius = level_safe_parse_float(param, "cash_radius");
  } else if (obj_id == obj_id_bullet) {
    arg.bullet.friendly = level_safe_parse_int(param, "friendly");
    arg.bullet.damage = level_safe_parse_float(param, "damage");
  } else if (obj_id == obj_id_tank) {
    arg.tank.max_hp = level_safe_parse_int(param, "max_hp");
    arg.tank.coins = level_safe_parse_int(param, "coins");
  } else if (obj_id == obj_id_factory) {
    arg.factory.max_tanks = level_safe_parse_int(param, "max_tanks");
    arg.factory.max_hp = level_safe_parse_float(param, "max_hp");
    arg.factory.spawn_delay = level_safe_parse_float(param, "spawn_delay");
    arg.factory.coins = level_safe_parse_int(param, "coins");
    object_spawn = cJSON_GetObjectItem(param, "object_spawn");
    arg.factory.type = object_by_name(level_safe_parse_char(object_spawn, "type"));    strcpy(arg.factory.param_name, level_safe_parse_char(param, "param_name"));
    strcpy(arg.factory.type_name, level_safe_parse_char(param, "type_name"));
    strcpy(arg.factory.sprite_name, level_safe_parse_char(param, "sprite_name"));
    strcpy(arg.factory.shape_name, level_safe_parse_char(param, "shape_name"));
  } else if (obj_id == obj_id_turret) {
    arg.turret.max_hp = level_safe_parse_int(param, "max_hp");
    arg.turret.coins = level_safe_parse_int(param, "coins");
    arg.turret.rot_speed = level_safe_parse_float(param, "rot_speed");
    arg.turret.shoot_interval = level_safe_parse_float(param, "shoot_interval");
    arg.turret.burst_number = level_safe_parse_int(param, "burst_number");
    arg.turret.tex_id = level_safe_parse_int(param, "tex_id");
    strcpy(arg.turret.shape_name, level_safe_parse_char(param, "shape_name"));
  } else if (obj_id == obj_id_rocket) {
    arg.rocket.max_hp = level_safe_parse_float(param, "max_hp");
    arg.rocket.coins = level_safe_parse_int(param, "coins");
    arg.rocket.tex_id = level_safe_parse_int(param, "tex_id");
    arg.rocket.force = level_safe_parse_float(param, "force");
    arg.rocket.damage = level_safe_parse_float(param, "damage");
  } else if (obj_id == obj_id_robotarm) {
    arg.robotarm.max_hp = level_safe_parse_int(param, "max_hp");
    arg.robotarm.coins = level_safe_parse_int(param, "coins");
  } else if (obj_id == obj_id_spikeball) {
    arg.spikeball.radius = level_safe_parse_float(param, "radius");
    arg.spikeball.up_speed = level_safe_parse_float(param, "up_speed");
    arg.spikeball.down_force = level_safe_parse_float(param, "down_force");
    arg.spikeball.top_delay = level_safe_parse_float(param, "top_delay");
    arg.spikeball.bot_delay = level_safe_parse_float(param, "bot_delay");
    arg.spikeball.damage = level_safe_parse_float(param, "damage");
  } else if (obj_id == obj_id_coin) {
    arg.coin.explo_fmax = level_safe_parse_float(param, "explo_fmax");
  }
const int paramsize = obj_id->P_SIZE;
 void * data = calloc(1, paramsize);
memcpy(data, &arg, paramsize);
return data;}