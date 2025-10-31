#include "rotation.h"
#include <stdlib.h>

void rotate_flip(struct image* img) {
    for (size_t row = 0; row < img->height; row++) {
        for (size_t col = 0; col < img->width / 2; col++) {
            size_t left_idx = row * img->width + col;
            size_t right_idx = row * img->width + (img->width - col - 1);

            struct pixel temp = get_pixel(*img, left_idx);
            set_pixel(img, left_idx, get_pixel(*img, right_idx));
            set_pixel(img, right_idx, temp);
        }
    }
}

void rotate_image(struct image* img, int direction) {
    struct image rotated_img = {0};
    rotated_img.width = img->height;
    rotated_img.height = img->width;
    rotated_img.data = malloc(rotated_img.width * rotated_img.height * sizeof(struct pixel));

    for (size_t row = 0; row < img->height; row++) {
        for (size_t col = 0; col < img->width; col++) {
            size_t new_row, new_col;
            if (direction == 1) {
                new_row = col;
                new_col = img->height - row - 1;
            } else {
                new_row = img->width - col - 1;
                new_col = row;
            }
            size_t old_idx = row * img->width + col;
            size_t new_idx = new_row * rotated_img.width + new_col;
            set_pixel(&rotated_img, new_idx, get_pixel(*img, old_idx));
        }
    }

    free(img->data);
    img->data = rotated_img.data;
    img->width = rotated_img.width;
    img->height = rotated_img.height;
}

void rotate_ccw90(struct image* img) {
    rotate_image(img, 1);
}

void rotate_cw90(struct image* img) {
    rotate_image(img, 0);
}

void rotate_flipv(struct image* img) {
    rotate_flip(img);
    rotate_ccw90(img);
    rotate_ccw90(img);
}
