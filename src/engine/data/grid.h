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

typedef struct polgrid {
	int cols, rows; /* number of rows in selected region */
	float cosxcol[GRID_MAXCOL];
	float sinxcol[GRID_MAXCOL];

	float rad[GRID_MAXROW*2]; /* all computed radiuses as function of yth row */
	float min_rad; /* miniumum radius */
	float max_rows; /* max number of rows */
	int inner_i; /* inner index of selection */
	int outer_i; /* outer index of selection (exclusive) */

	float theta_unit; /* angle between to columns */
} polgrid;

typedef struct grid_index {
	int xcol, yrow;
	float dist_sq;
	float angle;
} grid_index;

inline static cpVect grid_getpos(polgrid *pgrid, int col_i, int row_i)
{
	return cpv(pgrid->cosxcol[col_i] * pgrid->rad[row_i], pgrid->sinxcol[col_i] * pgrid->rad[row_i]);
}

inline static int grid_wrap_index(polgrid *pgrid, int *x, int *y)
{
	int cols = pgrid->cols;
	int ii = pgrid->inner_i, oi = pgrid->outer_i;
	int outside = (*y >= oi-1) || (*y < ii);
    *y = (*y >= oi-1) ? (oi-2 > ii ? oi-2 : 0) : (*y < ii) ? ii : *y;
    *x = (*x >= cols) ? *x - cols : (*x < 0) ? *x + cols : *x;
    return outside;
}

inline static void grid_getpos2f(polgrid *pgrid, float *pos, int col_x, int row_y)
{
	float cols = pgrid->cols;
	float row_min = pgrid->inner_i;
	float row_max = pgrid->outer_i;
	row_y = (row_y >= row_max) ? row_max-1 : (row_y < row_min ? row_min : row_y);
	col_x = (col_x >= cols) ? col_x - cols : (col_x < 0 ? col_x + cols : col_x);
	if (col_x < 0 || col_x >= pgrid->cols) {
		fprintf(stderr, "ERROR: col_i > cols\n");
		exit(-1);
	}
	if (row_y < row_min || row_y >= row_max) {
		fprintf(stderr, "ERROR: row_i > rows\n");
		exit(-1);
	}
	float r = pgrid->rad[row_y];
	pos[0] = pgrid->cosxcol[col_x] * r;
	pos[1] = pgrid->sinxcol[col_x] * r;
}


inline static void grid_getpos2cpv(polgrid *pgrid, cpVect *pos, int col_x, int row_y)
{
	float cols = pgrid->cols;
	float row_min = pgrid->inner_i;
	float row_max = pgrid->outer_i;
	row_y = (row_y >= row_max) ? row_max-1 : (row_y < row_min ? row_min : row_y);
	col_x = (col_x >= cols) ? col_x - cols : (col_x < 0 ? col_x + cols : col_x);
	if (col_x < 0 || col_x >= pgrid->cols) {
		fprintf(stderr, "ERROR: col_i > cols\n");
		exit(-1);
	}
	if (row_y < row_min || row_y >= row_max) {
		fprintf(stderr, "ERROR: row_i > rows\n");
		exit(-1);
	}
	cpFloat r = pgrid->rad[row_y];
	pos->x = pgrid->cosxcol[col_x] * r;
	pos->y = pgrid->sinxcol[col_x] * r;
}
inline static void grid_getpos2cpv_direct(polgrid *pgrid, cpVect *pos, int col_x, int row_y)
{
	float cols = pgrid->cols;
	col_x = (col_x >= cols) ? col_x - cols : (col_x < 0 ? col_x + cols : col_x);
	cpFloat r = pgrid->rad[row_y];
	pos->x = pgrid->cosxcol[col_x] * r;
	pos->y = pgrid->sinxcol[col_x] * r;
}


inline static void grid_getquad8f(polgrid *pgrid, float *quad, int col_x, int row_y)
{
	grid_getpos2f(pgrid, quad, col_x, row_y);
	grid_getpos2f(pgrid, quad+2, col_x+1, row_y);
	grid_getpos2f(pgrid, quad+4, col_x, row_y+1);
	grid_getpos2f(pgrid, quad+6, col_x+1, row_y+1);
}
inline static void grid_getquad8cpv(polgrid *pgrid, cpVect *quad, int col_x, int row_y)
{
	grid_getpos2cpv(pgrid, quad+0, col_x+0, row_y+0);
	grid_getpos2cpv(pgrid, quad+1, col_x+1, row_y+0);
	grid_getpos2cpv(pgrid, quad+2, col_x+1, row_y+1);
	grid_getpos2cpv(pgrid, quad+3, col_x+0, row_y+1);
}
inline static void grid_getquad8cpv_direct(polgrid *pgrid, cpVect *quad, int col_x, int row_y)
{
	grid_getpos2cpv_direct(pgrid, quad+0, col_x+0, row_y+0);
	grid_getpos2cpv_direct(pgrid, quad+1, col_x+1, row_y+0);
	grid_getpos2cpv_direct(pgrid, quad+2, col_x+1, row_y+1);
	grid_getpos2cpv_direct(pgrid, quad+3, col_x+0, row_y+1);
}

#define grid_inner_radius(grid_ptr) ((grid_ptr)->rad[(grid_ptr)->inner_i])
#define grid_outer_radius(grid_ptr) ((grid_ptr)->rad[(grid_ptr)->outer_i-1])

void grid_draw(polgrid *pgrid, int layer, float linewidth);
polgrid *grid_create(int col_count, float min_rad, int max_rows, int inner_index, int outer_index);
void grid_setcols(polgrid *pgrid, int cols);
void grid_setregion2i(polgrid *pgrid, int inner_index, int outer_index);
void grid_setregion2f(polgrid *pgrid, float inner_radius, int outer_radius);
void grid_free(polgrid *pgrid);
grid_index grid_getindex(polgrid *pgrid, cpVect pos);
#endif /* GRID_H_ */
