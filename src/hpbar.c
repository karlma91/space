#include "hpbar.h"
#include "draw.h"
#include "chipmunk.h"
#include "main.h"

void hpbar_init(hpbar *hp_bar, float max_hp, float width, float height, float x_offset, float y_offset, cpVect *pos)
{
	hpbar values = {
			.value = max_hp,
			.max_hp = max_hp,
			.draw_value = 1,
			.hp_timer = 0,
			.width = width,
			.height = height,
			.x_offset = x_offset,
			.y_offset = y_offset,
			.x = &(pos->x),
			.y = &(pos->y)
	};

	*hp_bar = values;
}

void hpbar_draw(hpbar *hp_bar)
{
	float hp_percent = hp_bar->value / hp_bar->max_hp;
	hp_percent = hp_percent < 0 ? 0 : hp_percent > 1 ? 1 : hp_percent;
	float hp_dt = dt;

	if (hp_bar->draw_value != hp_percent) {
		if (hp_bar->draw_value - hp_dt > hp_percent) {
			hp_bar->draw_value -= hp_dt;
		} else if (hp_bar->draw_value + hp_dt < hp_percent) {
			hp_bar->draw_value += hp_dt;
		} else {
			hp_bar->draw_value = hp_percent;
		}

		hp_bar->hp_timer = 2;
	}

	if ((hp_bar->hp_timer > 0 || hp_percent < 0.5) && hp_bar->draw_value > 0) {
		hp_bar->hp_timer -= dt;
		draw_bar(*(hp_bar->x) + hp_bar->x_offset, *(hp_bar->y) + hp_bar->y_offset, hp_bar->width, hp_bar->height, hp_bar->draw_value, hp_percent);
	}
}

