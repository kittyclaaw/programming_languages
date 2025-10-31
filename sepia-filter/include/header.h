#ifndef HEADER_H
#define HEADER_H

#include "image.h"
#include <stdint.h>
#include <stdio.h>


#define BF_TYPE_DEFAULT 0x4D42
#define BI_BIT_COUNT_DEFAULT 24
#define BI_PLANES_DEFAULT 1
#define BI_SIZE_DEFAULT 40

// BMP Header структура
struct __attribute__((packed)) header_bmp {
    uint16_t bfType;
    uint32_t bfileSize;
    uint32_t bfReserved;
    uint32_t bOffBits;
    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};

// Статус коды чтения BMP
enum read_status {
    READ_OK = 0,                // Successfully read
    READ_ERROR,                 // General read error
    READ_INVALID_SIGNATURE = 1, // Invalid file signature
    READ_INVALID_BITS = 12,     // Unsupported bit count
    READ_INVALID_HEADER = 2     // Invalid BMP header
};

// Статус коды записи BMP
enum write_status {
    WRITE_OK = 0,   // Successfully written
    WRITE_ERROR,    // General write error
};


struct header_info {
    struct header_bmp header;
    enum read_status status;
};


struct header_bmp header_create(uint64_t width, uint64_t height);
struct header_info header_read(FILE* in);

#endif
