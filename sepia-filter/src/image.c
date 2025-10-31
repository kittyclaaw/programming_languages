#include "../include/image.h"
#include <stdlib.h>

struct image* create_image(uint64_t sourceWidth, uint64_t sourceHeight) {
    if (sourceWidth == 0 || sourceHeight == 0) {
        return NULL;
    }

    struct image *res = malloc(sizeof(struct image));
    if (res == NULL) {
        return NULL;
    }

    res->data = malloc(sourceWidth * sourceHeight * sizeof(struct pixel));
    if (res->data == NULL) {
        free(res);
        return NULL;
    }

    res->width = sourceWidth;
    res->height = sourceHeight;
    return res;
}

void free_image(struct image *image) {
    if (image != NULL) {
        free(image->data);
        image->data = NULL;
        free(image);
    }
}
