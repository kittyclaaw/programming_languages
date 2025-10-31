

#ifndef TRANSFORM_H
#define TRANSFORM_H
#include "image.h"

struct image* rotate(struct image const* source);
struct image* sepia_c(struct image const* source);
struct image* sepia_asm(struct image const* source);
#endif
