#include "we.h"
#include "grid.h"

#define GRID_CELLRATIO 0.5

void grid_update(polgrid *pgrid, float col_count, float inn_rad, float out_rad)
{
	pgrid->rows = 0;
	pgrid->cols = col_count;
	pgrid->irad = inn_rad;
	pgrid->orad = out_rad;
	pgrid->theta_unit = (WE_2PI * 1 / pgrid->cols);

	float c_height = pgrid->irad;
	float t_height = pgrid->orad;
	while (c_height < t_height) {
		pgrid->rad[pgrid->rows] = c_height;
		c_height += pgrid->theta_unit * c_height * GRID_CELLRATIO;
		pgrid->rows++;
	}
	pgrid->orad = c_height;

	int i;
	for (i = 0; i < pgrid->cols; i++) {
		float theta = pgrid->theta_unit * i;
		pgrid->cosxcol[i] = cosf(theta);
		pgrid->sinxcol[i] = sinf(theta);
	}
}

void grid_draw(polgrid *pgrid, int layer, float linewidth)
{
	return;
	int i, j;
	float circle[(1+pgrid->cols)*2];
	float line[pgrid->rows*2];
	for (j = 0; j < pgrid->rows; j++) {
		for (i = 0; i < pgrid->cols; i++) {
			grid_getpos2f(pgrid, &circle[2*i], i, j);
		}
		grid_getpos2f(pgrid, &circle[2*i], 0, j);
		draw_line_strip(circle, (pgrid->cols+1)*2, linewidth);
	}
	for (i = 0; i < pgrid->cols; i++) {
		for (j = 0; j < pgrid->rows; j++) {
			grid_getpos2f(pgrid, &line[2*j], i, j);
		}
		draw_line_strip(line, pgrid->rows*2, linewidth);
	}
}

polgrid *grid_create(int col_count, float inn_rad, float out_rad)
{
	polgrid *pgrid = calloc(1, sizeof *pgrid);
	grid_update(pgrid, col_count, inn_rad, out_rad);
	return pgrid;
}

grid_index grid_getindex(polgrid *pgrid, cpVect pos)
{
	grid_index grid_i;
	float angle = cpvtoangle(pos);
	angle = angle < 0 ? angle + WE_2PI : angle;
	grid_i.xcol = (int) (angle / pgrid->theta_unit);
	grid_i.yrow = -1;
	float radsq = cpvlengthsq(pos);

	int y;
	float r1 = pgrid->rad[0]; r1 *= r1;
	for (y = 1; y < pgrid->rows; y++)
	{
		float r2 = pgrid->rad[y]; r2 *= r2;
		if (radsq >= r1 && radsq < r2) {
			grid_i.yrow = y-1;
			break;
		}
		r1 = r2;
	}

	return grid_i;

}

void grid_free(polgrid *pgrid)
{
	free(pgrid);
}

