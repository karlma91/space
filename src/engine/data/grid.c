#include "we.h"
#include "grid.h"

#define GRID_CELLRATIO 0.5

void grid_setrows(polgrid *pgrid, float min_rad, int max_rows)
{
	int row = 0;
	float radius = min_rad;
	pgrid->min_rad = min_rad;
	pgrid->max_rows = max_rows;
	while (row < max_rows) {
		pgrid->rad[row] = radius;
		radius += radius * (pgrid->theta_unit * GRID_CELLRATIO);
		row++;
	}
}

void grid_setcols(polgrid *pgrid, int cols)
{
	int i;
	pgrid->cols = cols;
	pgrid->theta_unit = (WE_2PI * 1 / pgrid->cols);
	for (i = 0; i < pgrid->cols; i++) {
		float theta = pgrid->theta_unit * i;
		pgrid->cosxcol[i] = cosf(theta);
		pgrid->sinxcol[i] = sinf(theta);
	}
}

void grid_setregion2i(polgrid *pgrid, int inner_index, int outer_index)
{
	pgrid->inner_i = inner_index;
	pgrid->outer_i = outer_index;
	pgrid->rows = outer_index - inner_index;
}

void grid_setregion2f(polgrid *pgrid, float inner_radius, int outer_radius)
{
	int y;
	int ii = 0, oi = 0;
	float r_last = pgrid->rad[0];
	for (y = 0; y < pgrid->max_rows; y++) {
		float r = pgrid->rad[y];
		float diff = (r - r_last) / 2;
		ii = (r - diff <= inner_radius) ? y : ii;
		oi = (r - 3*diff <= outer_radius) ? y : oi;
		r_last = r;
	}
	grid_setregion2i(pgrid, ii, oi);
}

void grid_draw(polgrid *pgrid, int layer, float linewidth)
{
	int i, j;
	float circle[(1+pgrid->cols)*2];
	float line[4];
	for (j = pgrid->inner_i; j < pgrid->outer_i; j++) {
		for (i = 0; i < pgrid->cols; i++) {
			grid_getpos2f(pgrid, &circle[2*i], i, j);
		}
		grid_getpos2f(pgrid, &circle[2*i], 0, j);
		draw_line_strip(circle, (pgrid->cols+1)*2, linewidth);
	}
	for (i = 0; i < pgrid->cols; i++) {
		grid_getpos2f(pgrid, &line[0], i, pgrid->inner_i);
		grid_getpos2f(pgrid, &line[2], i, pgrid->outer_i);
		//draw_line_strip(line, 4, linewidth);
		draw_quad_line(0, cpv(line[0],line[1]),cpv(line[2],line[3]),linewidth/4);
	}
}

polgrid *grid_create(int col_count, float min_rad, int max_rows, int inner_index, int outer_index)
{
	polgrid *pgrid = calloc(1, sizeof *pgrid);
	grid_setcols(pgrid, col_count);
	grid_setrows(pgrid, min_rad, max_rows);
	grid_setregion2i(pgrid, inner_index, outer_index);
	return pgrid;
}

grid_index grid_getindex(polgrid *pgrid, cpVect pos)
{
	grid_index grid_i;
	grid_i.angle = cpvtoangle(pos);
	grid_i.angle = grid_i.angle < 0 ? grid_i.angle + WE_2PI : grid_i.angle;
	grid_i.xcol = (int) (grid_i.angle / pgrid->theta_unit);
	grid_i.yrow = -1;
	float radsq = cpvlengthsq(pos);
	grid_i.dist_sq = radsq;

	int y;
	float r1 = pgrid->rad[pgrid->inner_i]; r1 *= r1;
	for (y = pgrid->inner_i + 1; y < pgrid->outer_i; y++) {
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

