#ifndef ROTATE_H
#define ROTATE_H

#include "image.h"

void rotate_flip(struct image* img);
void rotate_flipv(struct image* img);
void rotate_ccw90(struct image* img);
void rotate_cw90(struct image* img);

#endif
