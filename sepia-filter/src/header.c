#include "../include/header.h"

struct header_bmp header_create(uint64_t width, uint64_t height) {
    struct header_bmp header = {0}; // Инициализируем все поля структуры нулями

    header.bfType = BF_TYPE_DEFAULT;
    header.biSize = BI_SIZE_DEFAULT;
    header.biWidth = width;
    header.biHeight = height;
    header.biPlanes = BI_PLANES_DEFAULT;
    header.biBitCount = BI_BIT_COUNT_DEFAULT;

    uint64_t pixel_data_size = width * height * (BI_BIT_COUNT_DEFAULT / 8);
    header.bfileSize = sizeof(struct header_bmp) + pixel_data_size;
    header.bfReserved = 0;
    header.bOffBits = sizeof(struct header_bmp);
    header.biCompression = 0;
    header.biSizeImage = pixel_data_size;
    header.biXPelsPerMeter = 0;
    header.biYPelsPerMeter = 0;
    header.biClrUsed = 0;
    header.biClrImportant = 0;

    return header;
}

struct header_info header_read(FILE *in) {
    struct header_info result = {0}; // Инициализируем структуру нулями
    result.status = READ_OK;

    if (!in) {
        result.status = READ_ERROR; // Ошибка: файл не открыт
        return result;
    }

    if (fread(&result.header, sizeof(struct header_bmp), 1, in) != 1) {
        result.status = READ_ERROR; // Ошибка при чтении данных
    }

    // Проверка корректности формата BMP
    if (result.header.bfType != BF_TYPE_DEFAULT) {
        result.status = READ_INVALID_SIGNATURE;
    }

    return result;
}
