/*
 * grid.h
 *
 *  Created on: 14. jan. 2014
 *      Author: mathiahw
 */

#ifndef GRID_H_
#define GRID_H_

#define GRID_MAXCOL 400
#define GRID_MAXROW 400

typedef enum GRID_TYPE {
	GRID_CARTES,
	GRID_POL
} GRID_TYPE;

typedef struct grid_pol {
	GRID_TYPE type;
	int cols, rows; /* number of rows in selected region */
	float min_rad; /* miniumum radius */
	float max_rows; /* max number of rows */
	int inner_i; /* inner index of selection */
	int outer_i; /* outer index of selection (exclusive) */
	float cosxcol[GRID_MAXCOL];
	float sinxcol[GRID_MAXCOL];
	float rad[GRID_MAXROW*2]; /* all computed radiuses as function of yth row */

	float theta_unit; /* angle between to columns */
} grid_pol;

typedef struct grid_cart {
	GRID_TYPE type;
	int cols, rows; /* number of rows in selected region */
	float min_rad; /* miniumum radius */
	float max_rows; /* max number of rows */
} grid_cart;

typedef union we_grid {
	GRID_TYPE type;
	grid_cart cart;
	grid_pol pol;
} we_grid;

typedef struct grid_index {
	int xcol, yrow;
	float dist_sq;
	float angle;
} grid_index;

inline static cpVect grid_getpos(we_grid *grid, int col_i, int row_i)
{
	return cpv(grid->pol.cosxcol[col_i] * grid->pol.rad[row_i], grid->pol.sinxcol[col_i] * grid->pol.rad[row_i]);
}

inline static int grid_wrap_index(we_grid *grid, int *x, int *y)
{
	int cols = grid->pol.cols;
	int ii = grid->pol.inner_i, oi = grid->pol.outer_i;
	int outside = (*y >= oi-1) || (*y < ii);
    *y = (*y >= oi-1) ? (oi-2 > ii ? oi-2 : 0) : (*y < ii) ? ii : *y;
    *x = (*x >= cols) ? *x - cols : (*x < 0) ? *x + cols : *x;
    return outside;
}

inline static void grid_getpos2f(we_grid *grid, float *pos, int col_x, int row_y)
{
	float cols = grid->pol.cols;
	float row_min = grid->pol.inner_i;
	float row_max = grid->pol.outer_i;
	row_y = (row_y >= row_max) ? row_max-1 : (row_y < row_min ? row_min : row_y);
	col_x = (col_x >= cols) ? col_x - cols : (col_x < 0 ? col_x + cols : col_x);
	if (col_x < 0 || col_x >= grid->pol.cols) {
		fprintf(stderr, "ERROR: col_i > cols\n");
		exit(-1);
	}
	if (row_y < row_min || row_y >= row_max) {
		fprintf(stderr, "ERROR: row_i > rows\n");
		exit(-1);
	}
	float r = grid->pol.rad[row_y];
	pos[0] = grid->pol.cosxcol[col_x] * r;
	pos[1] = grid->pol.sinxcol[col_x] * r;
}


inline static void grid_getpos2cpv(we_grid *grid, cpVect *pos, int col_x, int row_y)
{
	float cols = grid->pol.cols;
	float row_min = grid->pol.inner_i;
	float row_max = grid->pol.outer_i;
	row_y = (row_y >= row_max) ? row_max-1 : (row_y < row_min ? row_min : row_y);
	col_x = (col_x >= cols) ? col_x - cols : (col_x < 0 ? col_x + cols : col_x);
	if (col_x < 0 || col_x >= grid->pol.cols) {
		fprintf(stderr, "ERROR: col_i > cols\n");
		exit(-1);
	}
	if (row_y < row_min || row_y >= row_max) {
		fprintf(stderr, "ERROR: row_i > rows\n");
		exit(-1);
	}
	cpFloat r = grid->pol.rad[row_y];
	pos->x = grid->pol.cosxcol[col_x] * r;
	pos->y = grid->pol.sinxcol[col_x] * r;
}
inline static void grid_getpos2cpv_direct(we_grid *grid, cpVect *pos, int col_x, int row_y)
{
	float cols = grid->pol.cols;
	col_x = (col_x >= cols) ? col_x - cols : (col_x < 0 ? col_x + cols : col_x);
	cpFloat r = grid->pol.rad[row_y];
	pos->x = grid->pol.cosxcol[col_x] * r;
	pos->y = grid->pol.sinxcol[col_x] * r;
}


inline static void grid_getquad8f(we_grid *grid, float *quad, int col_x, int row_y)
{
	grid_getpos2f(grid, quad, col_x, row_y);
	grid_getpos2f(grid, quad+2, col_x+1, row_y);
	grid_getpos2f(grid, quad+4, col_x, row_y+1);
	grid_getpos2f(grid, quad+6, col_x+1, row_y+1);
}
inline static void grid_getquad8cpv(we_grid *grid, cpVect *quad, int col_x, int row_y)
{
	grid_getpos2cpv(grid, quad+0, col_x+0, row_y+0);
	grid_getpos2cpv(grid, quad+1, col_x+1, row_y+0);
	grid_getpos2cpv(grid, quad+2, col_x+1, row_y+1);
	grid_getpos2cpv(grid, quad+3, col_x+0, row_y+1);
}
inline static void grid_getquad8cpv_direct(we_grid *grid, cpVect *quad, int col_x, int row_y)
{
	grid_getpos2cpv_direct(grid, quad+0, col_x+0, row_y+0);
	grid_getpos2cpv_direct(grid, quad+1, col_x+1, row_y+0);
	grid_getpos2cpv_direct(grid, quad+2, col_x+1, row_y+1);
	grid_getpos2cpv_direct(grid, quad+3, col_x+0, row_y+1);
}

#define grid_inner_radius(grid_ptr) ((grid_ptr)->pol.rad[(grid_ptr)->pol.inner_i])
#define grid_outer_radius(grid_ptr) ((grid_ptr)->pol.rad[(grid_ptr)->pol.outer_i-1])

void grid_draw(we_grid *grid, int layer, float linewidth);
we_grid *grid_create(int col_count, float min_rad, int max_rows, int inner_index, int outer_index);
void grid_setcols(we_grid *grid, int cols);
void grid_setregion2i(we_grid *grid, int inner_index, int outer_index);
void grid_setregion2f(we_grid *grid, float inner_radius, int outer_radius);
void grid_free(we_grid *grid);
grid_index grid_getindex(we_grid *grid, cpVect pos);
#endif /* GRID_H_ */
