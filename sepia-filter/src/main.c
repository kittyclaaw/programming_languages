#include "../include/io_bmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

void print_usage_and_exit(const char *program_name) {
    fprintf(stderr, "Usage: %s <input file> <output file> <r|a|c>\n", program_name);
    fprintf(stderr, "Modes:\n");
    fprintf(stderr, "  r - Rotate\n");
    fprintf(stderr, "  c - Sepia (C implementation)\n");
    fprintf(stderr, "  a - Sepia (ASM implementation)\n");
    exit(EXIT_FAILURE);
}

long get_time_in_microseconds() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_utime.tv_sec * 1000000 + usage.ru_utime.tv_usec;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        print_usage_and_exit(argv[0]);
    }

    char mode = argv[3][0];
    if (mode != 'r' && mode != 'c' && mode != 'a') {
        fprintf(stderr, "Invalid mode.\n");
        print_usage_and_exit(argv[0]);
    }

    FILE *input_file = fopen(argv[1], "rb");
    if (!input_file) {
        fprintf(stderr, "Error: Could not open input file '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }

    struct image *input_image = malloc(sizeof(struct image));
    if (!input_image) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(input_file);
        return EXIT_FAILURE;
    }

    enum read_status read_status = from_bmp(input_file, input_image);
    fclose(input_file);

    if (read_status != READ_OK) {
        fprintf(stderr, "Error: Failed to read BMP file\n");
        free(input_image);
        return EXIT_FAILURE;
    }

    long start_time = get_time_in_microseconds();

    struct image *output_image = NULL;
    switch (mode) {
        case 'r':
            output_image = rotate(input_image);
            break;
        case 'c':
            output_image = sepia_c(input_image);
            break;
        case 'a':
            output_image = sepia_asm(input_image);
            break;
        default:
            fprintf(stderr, "Error: Unsupported mode\n");
            free_image(input_image);
            return EXIT_FAILURE;
    }

    if (!output_image) {
        fprintf(stderr, "Error: Transformation failed\n");
        free_image(input_image);
        return EXIT_FAILURE;
    }

    long elapsed_time = get_time_in_microseconds() - start_time;
    printf("Transformation took %ld microseconds\n", elapsed_time);

    FILE *output_file = fopen(argv[2], "wb");
    if (!output_file) {
        fprintf(stderr, "Error: Could not open output file '%s'\n", argv[2]);
        free_image(input_image);
        free_image(output_image);
        return EXIT_FAILURE;
    }

    enum write_status write_status = to_bmp(output_file, output_image);
    fclose(output_file);

    if (write_status != WRITE_OK) {

        fprintf(stderr, "Error: Failed to write BMP file\n");
        free_image(input_image);
        free_image(output_image);
        return EXIT_FAILURE;
    }

    free_image(input_image);
    free_image(output_image);

    printf("Success!\n");
    return EXIT_SUCCESS;
}
