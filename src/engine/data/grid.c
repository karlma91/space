#include "we.h"
#include "grid.h"

#define GRID_CELLRATIO 1//0.5

void grid_setrows(we_grid *grid, float min_rad, int max_rows)
{
	int row = 0;
	float radius = min_rad;
	grid->pol.min_rad = min_rad;
	grid->pol.max_rows = max_rows;
	while (row < max_rows) {
		grid->pol.rad[row] = radius;
		radius += radius * (grid->pol.theta_unit * GRID_CELLRATIO);
		row++;
	}
}

void grid_setcols(we_grid *grid, int cols)
{
	int i;
	grid->pol.cols = cols;
	grid->pol.theta_unit = (WE_2PI * 1 / grid->pol.cols);
	for (i = 0; i < grid->pol.cols; i++) {
		float theta = grid->pol.theta_unit * i;
		grid->pol.cosxcol[i] = cosf(theta);
		grid->pol.sinxcol[i] = sinf(theta);
	}
}

void grid_setregion2i(we_grid *grid, int inner_index, int outer_index)
{
	grid->pol.inner_i = inner_index;
	grid->pol.outer_i = outer_index;
	grid->pol.rows = outer_index - inner_index;
}

void grid_setregion2f(we_grid *grid, float inner_radius, int outer_radius)
{
	int y;
	int ii = 0, oi = 0;
	float r_last = grid->pol.rad[0];
	for (y = 0; y < grid->pol.max_rows; y++) {
		float r = grid->pol.rad[y];
		float diff = (r - r_last) / 2;
		ii = (r - diff <= inner_radius) ? y : ii;
		oi = (r - 3*diff <= outer_radius) ? y : oi;
		r_last = r;
	}
	grid_setregion2i(grid, ii, oi);
}

void grid_draw(we_grid *grid, int layer, float linewidth)
{
	int i, j;
	float circle[(1+grid->pol.cols)*2];
	float line[4];
	for (j = grid->pol.inner_i; j < grid->pol.outer_i; j++) {
		for (i = 0; i < grid->pol.cols; i++) {
			grid_getpos2f(grid, &circle[2*i], i, j);
		}
		grid_getpos2f(grid, &circle[2*i], 0, j);
		draw_line_strip(circle, (grid->pol.cols+1)*2, linewidth);
	}
	for (i = 0; i < grid->pol.cols; i++) {
		grid_getpos2f(grid, &line[0], i, grid->pol.inner_i);
		grid_getpos2f(grid, &line[2], i, grid->pol.outer_i);
		//draw_line_strip(line, 4, linewidth);
		draw_quad_line(0, cpv(line[0],line[1]),cpv(line[2],line[3]),linewidth/4);
	}
}

we_grid *grid_create(int col_count, float min_rad, int max_rows, int inner_index, int outer_index)
{
	we_grid *grid = calloc(1, sizeof *grid);
	grid->type = GRID_POL;
	grid_setcols(grid, col_count);
	grid_setrows(grid, min_rad, max_rows);
	grid_setregion2i(grid, inner_index, outer_index);
	return grid;
}

grid_index grid_getindex(we_grid *grid, cpVect pos)
{
	grid_index grid_i;
	grid_i.angle = cpvtoangle(pos);
	grid_i.angle = grid_i.angle < 0 ? grid_i.angle + WE_2PI : grid_i.angle;
	grid_i.xcol = (int) (grid_i.angle / grid->pol.theta_unit);
	grid_i.yrow = -1;
	float radsq = cpvlengthsq(pos);
	grid_i.dist_sq = radsq;

	int y;
	float r1 = grid->pol.rad[grid->pol.inner_i]; r1 *= r1;
	for (y = grid->pol.inner_i + 1; y < grid->pol.outer_i; y++) {
		float r2 = grid->pol.rad[y]; r2 *= r2;
		if (radsq >= r1 && radsq < r2) {
			grid_i.yrow = y-1;
			break;
		}
		r1 = r2;
	}
	return grid_i;
}

void grid_free(we_grid *grid)
{
	free(grid);
}

