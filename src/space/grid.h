/*
 * grid.h
 *
 *  Created on: 14. jan. 2014
 *      Author: mathiahw
 */

#ifndef GRID_H_
#define GRID_H_

#define GRID_MAXCOL 100
#define GRID_MAXROW 100

typedef struct polgrid {
	int cols, rows;
	float cosxcol[GRID_MAXCOL];
	float sinxcol[GRID_MAXCOL];
	float rad[GRID_MAXROW*2];
	float irad; /* inner radius */
	float orad; /* outer radius */
	float theta_unit; /* angle between to columns */
} polgrid;

typedef struct grid_index {
	int xcol, yrow;
} grid_index;

inline static cpVect grid_getpos(polgrid *pgrid, int col_i, int row_i)
{
	return cpv(pgrid->cosxcol[col_i] * pgrid->rad[row_i], pgrid->sinxcol[col_i] * pgrid->rad[row_i]);
}

inline static void grid_getpos2f(polgrid *pgrid, float *pos, int col_i, int row_i)
{
	if (col_i < 0 || col_i >= pgrid->cols) {
		fprintf(stderr, "ERROR: col_i > cols\n");
		exit(-1);
	}
	if (row_i < 0 || row_i >= pgrid->rows) {
		fprintf(stderr, "ERROR: row_i > rows\n");
		exit(-1);
	}
	float r = pgrid->rad[row_i];
	pos[0] = pgrid->cosxcol[col_i] * r;
	pos[1] = pgrid->sinxcol[col_i] * r;
}

void grid_draw(polgrid *pgrid, int layer, float linewidth);
polgrid *grid_create(int col_count, float inn_rad, float out_rad);
void grid_update(polgrid *pgrid, float col_count, float inn_rad, float out_rad);
void grid_free(polgrid *pgrid);
grid_index grid_getindex(polgrid *pgrid, cpVect pos);
#endif /* GRID_H_ */
