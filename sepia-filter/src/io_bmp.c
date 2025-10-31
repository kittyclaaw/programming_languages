
#include "../include/header.h"
#include "../include/image.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BMP_SIGNATURE 0x4D42
#define BMP_HEADER_SIZE 54
#define DIB_HEADER_SIZE 40
#define VALID_BI_PLANES 1

uint8_t calc_padding(uint64_t width, uint8_t pixel_size)
{
    return (4 - (width * pixel_size) % 4) % 4;
}

// static void free_image(struct image* img)
// {
//     free(img->data);
//     img->data = NULL;
//
//     img->width = img->height = 0;
// }

static enum read_status read_row(FILE* in, struct pixel* row, size_t width, uint8_t padding)
{
    if (fread(row, sizeof(struct pixel), width, in) != width)
    {
        return READ_INVALID_BITS;
    }
    if (padding > 0 && fseek(in, padding, SEEK_CUR) == -1)
    {
        return READ_INVALID_BITS;
    }
    return READ_OK;
}

static enum write_status write_row(FILE* out, const struct pixel* row, size_t width, uint8_t padding)
{
    static const uint8_t padding_bytes[3] = {0, 0, 0};
    if (fwrite(row, sizeof(struct pixel), width, out) != width)
    {
        return WRITE_ERROR;
    }
    if (padding > 0 && fwrite(padding_bytes, 1, padding, out) != padding)
    {
        return WRITE_ERROR;
    }
    return WRITE_OK;
}

enum read_status from_bmp(FILE* in, struct image* img)
{
    struct header_bmp head;
    if (fread(&head, sizeof(head), 1, in) != 1)
    {
        return READ_INVALID_HEADER;
    }

    if (head.bfType != BMP_SIGNATURE || head.biPlanes != VALID_BI_PLANES || head.biSize != DIB_HEADER_SIZE)
    {
        return READ_INVALID_HEADER;
    }

    img->width = head.biWidth;
    img->height = head.biHeight;

    img->data = malloc(sizeof(struct pixel) * img->width * img->height);
    if (!img->data)
    {
        return READ_INVALID_BITS;
    }

    if (fseek(in, head.bOffBits, SEEK_SET) == -1)
    {
        free_image(img);
        return READ_INVALID_HEADER;
    }

    const uint8_t padding = calc_padding(img->width, sizeof(struct pixel));

    for (size_t i = 0; i < img->height; i++)
    {
        struct pixel* row = img->data + i * img->width;
        enum read_status row_status = read_row(in, row, img->width, padding);
        if (row_status != READ_OK)
        {
            free_image(img);
            return row_status;
        }
    }

    return READ_OK;
}

enum write_status to_bmp(FILE* out, const struct image* img)
{
    const uint8_t padding = calc_padding(img->width, sizeof(struct pixel));
    const uint32_t image_size = (img->width * sizeof(struct pixel) + padding) * img->height;

    const struct header_bmp head = {
        .bfType = BMP_SIGNATURE,
        .bfileSize = BMP_HEADER_SIZE + image_size,
        .bfReserved = 0,
        .bOffBits = BMP_HEADER_SIZE,
        .biSize = DIB_HEADER_SIZE,
        .biWidth = img->width,
        .biHeight = img->height,
        .biPlanes = 1,
        .biBitCount = sizeof(struct pixel) * 8,
        .biCompression = 0,
        .biSizeImage = image_size,
        .biXPelsPerMeter = 0,
        .biYPelsPerMeter = 0,
        .biClrUsed = 0,
        .biClrImportant = 0
    };

    if (fwrite(&head, sizeof(head), 1, out) != 1)
    {
        return WRITE_ERROR;
    }

    for (size_t i = 0; i < img->height; i++)
    {
        const struct pixel* row = img->data + i * img->width;
        enum write_status row_status = write_row(out, row, img->width, padding);
        if (row_status != WRITE_OK)
        {
            return row_status;
        }
    }

    return WRITE_OK;
}
