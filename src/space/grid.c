#include "we.h"
#include "grid.h"

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
		c_height += pgrid->theta_unit * c_height;
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

void grid_free(polgrid *pgrid)
{
	free(pgrid);
}

