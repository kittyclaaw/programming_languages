#include "image.h"

struct create_result create_image(const uint64_t width, const uint64_t height) {
    struct pixel* data = malloc(sizeof(struct pixel) * width * height);
    struct create_result res;

    if (data != NULL) {
        res.status = CREATED;
        res.img = (struct image) { width, height, data };
    } else {
        res.status = MALLOC_ERROR;
    }

    return res;
}

void destroy_image(struct image* const img) {
    img->height = 0;
    img->width = 0;
    free(img->data);
}


static int is_valid_index(const struct image* img, size_t pixel_index) {
    return img != NULL && pixel_index < img->width * img->height;
}


void set_pixel(struct image* img, const size_t pixel_index, const struct pixel new_pixel) {
    if (is_valid_index(img, pixel_index)) {
        img->data[pixel_index] = new_pixel;
    }
}


struct pixel get_pixel(struct image img, const size_t pixel_index) {
    struct pixel default_pixel = {0, 0, 0};
    if (is_valid_index(&img, pixel_index)) {
        return img.data[pixel_index];
    }
    return default_pixel;
}
