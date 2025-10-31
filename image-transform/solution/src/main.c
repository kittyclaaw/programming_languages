#include "image.h"
#include "bmp_format.h"
#include "rotation.h"
#include <stdio.h>
#include <string.h>

#define ARGUMENT_NUM 4

int main(int argc, char** argv) {
    if (argc < ARGUMENT_NUM) {
        fprintf(stderr, "3 arguments are required: <source-image> <transformed-image> <param>\n");
        return 1;
    }

    FILE* in = fopen(argv[1], "rb");
    if (in == NULL) {
        fprintf(stderr, "Error: Input file '%s' does not exist.\n", argv[1]);
        return 2;
    }

    struct image img = {0};
    enum read_status cur_read_status = from_bmp(in, &img);
    if (cur_read_status != READ_OK) {
        fprintf(stderr, "Failed to read from BMP file\n");
        fclose(in);
        return 12;
    }
    fclose(in);

    if (argc > 3) {
        switch (argv[3][0]) {
            case 'n':
                if (strcmp(argv[3], "none") == 0) {
                    printf("No transformation applied\n");
                    break;
                }
                goto unknown_param;
            case 'f':
                if (strcmp(argv[3], "fliph") == 0) {
                    rotate_flip(&img);
                    printf("Image flipped horizontally\n");
                } else if (strcmp(argv[3], "flipv") == 0) {
                    rotate_flipv(&img);
                    printf("Image flipped vertically and horizontally (180 degrees)\n");
                } else {
                    goto unknown_param;
                }
                break;
            case 'c':
                if (strcmp(argv[3], "ccw90") == 0) {
                    rotate_ccw90(&img);
                    printf("Image rotated 90 degrees counter-clockwise\n");
                } else if (strcmp(argv[3], "cw90") == 0) {
                    rotate_cw90(&img);
                    printf("Image rotated 90 degrees clockwise\n");
                } else {
                    goto unknown_param;
                }
                break;
            default:
            unknown_param:
                fprintf(stderr, "Unknown parameter: %s\n", argv[3]);
                return 1;
        }
    }

    FILE* out = fopen(argv[2], "wb");
    if (out == NULL) {
        fprintf(stderr, "Failed to open output file\n");
        return 1;
    }

    enum write_status cur_write_status = to_bmp(out, &img);
    if (cur_write_status != WRITE_OK) {
        fprintf(stderr, "Failed to write to BMP file\n");
        fclose(out);
        return 1;
    }

    fclose(out);


    if (img.data != NULL) {
        destroy_image(&img);
    }

    return 0;
}
