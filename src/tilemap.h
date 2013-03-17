/*
 * tilemap.h
 *
 *  Created on: 17. mars 2013
 *      Author: Karl
 */

#ifndef TILEMAP_H_
#define TILEMAP_H_

typedef struct tilemap {

    int total_width; /* total width in pixels */
    int total_height;

    int tile_width;
    int tile_height;

    int image_width;
    int image_height;
    int texture_id;

    int width; /* number of tiles */
    int height;

    int *data; /* tilmap data array */


}tilemap;


#endif /* TILEMAP_H_ */
