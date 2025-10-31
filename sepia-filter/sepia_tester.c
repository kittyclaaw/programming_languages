#include "include/io_bmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

long get_time_in_microseconds() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_utime.tv_sec * 1000000 + usage.ru_utime.tv_usec;
}

void run_tester(const char *input_file) {
    FILE *file = fopen(input_file, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open input file '%s'\n", input_file);
        exit(EXIT_FAILURE);
    }

    struct image *input_image = malloc(sizeof(struct image));
    if (!input_image) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    if (from_bmp(file, input_image) != READ_OK) {
        fprintf(stderr, "Error: Failed to read BMP file\n");
        fclose(file);
        free(input_image);
        exit(EXIT_FAILURE);
    }
    fclose(file);

    const int iterations = 100;
    long total_time_c = 0;
    long total_time_asm = 0;

    for (int i = 0; i < iterations; i++) {
        long start_time = get_time_in_microseconds();
        struct image *output_image_c = sepia_c(input_image);
        total_time_c += get_time_in_microseconds() - start_time;
        free_image(output_image_c);

        start_time = get_time_in_microseconds();
        struct image *output_image_asm = sepia_asm(input_image);
        total_time_asm += get_time_in_microseconds() - start_time;
        free_image(output_image_asm);
    }

    free_image(input_image);

    printf("Average time for sepia_c: %ld microseconds\n", total_time_c / iterations);
    printf("Average time for sepia_asm: %ld microseconds\n", total_time_asm / iterations);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    run_tester(argv[1]);

    return EXIT_SUCCESS;
}
