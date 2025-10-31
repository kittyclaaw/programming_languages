#ifndef IO_BMP_H
#define IO_BMP_H

#include "header.h"
#include "image.h"
#include "transform.h"
#include <stdio.h>


enum read_status from_bmp(FILE* in, struct image* img);
enum write_status to_bmp(FILE* out, struct image const* img);

#endif
