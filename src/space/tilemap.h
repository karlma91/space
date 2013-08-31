/*
 * tilemap.h
 *
 *  Created on: 17. mars 2013
 *      Author: Karl
 */

#ifndef TILEMAP_H_
#define TILEMAP_H_

#define MAX_SINGLE_TILE_COUNT 64

typedef struct tilemap {

    int total_width; /* total width in pixels */
    int total_height;

    int tile_width;
    int tile_height;

    int image_width;
    int image_height;
    int texture_id;

    int width; /* number of tiles in level */
    int height;

    float x2tc[MAX_SINGLE_TILE_COUNT];
    float y2tc[MAX_SINGLE_TILE_COUNT];

    //GLfloat **vertex;
    //GLfloat **textco;

    int *data; /* tilmap data array */

}tilemap;


void tilemap_render(tilemap *map);
int tilemap_create (tilemap *map, char *filename);
void tilemap_destroy(tilemap *map);


#endif /* TILEMAP_H_ */
