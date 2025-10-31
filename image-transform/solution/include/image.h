#include <stdint.h>
#include <stdlib.h>
#ifndef IMAGE_H
#define IMAGE_H

struct pixel { uint8_t b, g, r; };

struct image {
    uint64_t width, height;
    struct pixel* data;
};

enum  create_status {
    CREATED = 0,
    MALLOC_ERROR
};

struct create_result {
    enum create_status status;
    struct image img;
};

struct create_result create_image(uint64_t width, uint64_t height);
void destroy_image(struct image* img);
void set_pixel(struct image* img, size_t pixel_index, struct pixel new_pixel);
struct pixel get_pixel(struct image img, size_t pixel_index);

#endif
