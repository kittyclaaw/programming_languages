#include "../include/transform.h"
#include <stdint.h>
#include <stdlib.h>

#define MIN(x, y) ((x) < (y) ? (x) : (y))

static const float MATRIX[3][3] = {
    {0.393, 0.769, 0.189},
    {0.349, 0.686, 0.168},
    {0.272, 0.534, 0.131}
};

static struct pixel sepia_pixel(struct pixel pixel) {
    float sr = MIN(MATRIX[0][0] * pixel.r + MATRIX[0][1] * pixel.g + MATRIX[0][2] * pixel.b, 255);
    float sg = MIN(MATRIX[1][0] * pixel.r + MATRIX[1][1] * pixel.g + MATRIX[1][2] * pixel.b, 255);
    float sb = MIN(MATRIX[2][0] * pixel.r + MATRIX[2][1] * pixel.g + MATRIX[2][2] * pixel.b, 255);

    return (struct pixel) {
        .r = (uint8_t)sr,
        .g = (uint8_t)sg,
        .b = (uint8_t)sb
    };
}

struct image* rotate(const struct image* source) {
    if (!source) return NULL;

    struct image* result_img = create_image(source->height, source->width);
    if (!result_img) return NULL;

    for (uint64_t i = 0; i < source->height; i++) {
        for (uint64_t j = 0; j < source->width; j++) {
            size_t dest_idx = (source->height - i - 1) + j * source->height;
            size_t src_idx = i * source->width + j;
            result_img->data[dest_idx] = source->data[src_idx];
        }
    }

    return result_img;
}

struct image* sepia_c(const struct image* source) {
    if (!source) return NULL;

    struct image* result_img = create_image(source->width, source->height);
    if (!result_img) return NULL;

    size_t total_pixels = source->width * source->height;
    for (size_t i = 0; i < total_pixels; i++) {
        result_img->data[i] = sepia_pixel(source->data[i]);
    }

    return result_img;
}

extern void sepia_asm_helper(const struct pixel* src, struct pixel* dest);

struct image* sepia_asm(const struct image* source) {
    if (!source) return NULL;

    struct image* result_img = create_image(source->width, source->height);
    if (!result_img) return NULL;

    size_t total_pixels = source->width * source->height;
    size_t vector_pixels = (total_pixels / 4) * 4;

    for (size_t i = 0; i < vector_pixels; i += 4) {
        sepia_asm_helper(&source->data[i], &result_img->data[i]);
    }

    for (size_t i = vector_pixels; i < total_pixels; i++) {
        result_img->data[i] = sepia_pixel(source->data[i]);
    }

    return result_img;
}
