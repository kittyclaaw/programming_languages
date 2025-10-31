#include "image.h"
#include "bmp_format.h"

#define BF_TYPE 19778
#define BF_RESERVED 0
#define B_OFF_BITS 54
#define BI_SIZE 40
#define BI_PLANES 1
#define BI_BIT_COUNT 24
#define BI_COMPRESSION 0
#define BI_PELS_PER_METER 2834
#define BI_CLR_USED 0
#define BI_CLR_IMPORTANT 0
#define MULTIPLICITY 4

const char* read_status_string[] = {
	[READ_OK] = "Read successefully",
	[READ_INVALID_SIGNATURE] = "Read invalid signature",
	[READ_INVALID_BITS] = "Read invalid bits",
	[READ_INVALID_HEADER] = "Read invalid header",
	[FSEEK_ERROR] = "Failed to move stream pointer",
	[ALLOCATION_ERROR] = "Failed to allocate memory for the image"
};


const char* write_status_string[] = {
	[WRITE_OK] = "Wrote successefully",
	[WRITE_HEADER_ERROR] = "Failed to write header",
	[WRITE_PIXELS_ERROR] = "Failed to write pixels",
	[WRITE_PADDING_ERROR] = "Failed to write padding"
};

static uint8_t get_padding(uint64_t width) {
	uint8_t res = MULTIPLICITY - (width * sizeof(struct pixel) % MULTIPLICITY);
	return res % MULTIPLICITY;
}

enum read_status from_bmp(FILE* const in, struct image* const img) {

	struct bmp_header cur_header = {0};
	if (fread(&cur_header, sizeof(struct bmp_header), 1, in) != 1) {
		fprintf(stderr, "%s", read_status_string[READ_INVALID_HEADER]);
		return READ_INVALID_HEADER;
	}

	struct create_result res = create_image(cur_header.biWidth, cur_header.biHeight);
	if (res.status == CREATED) {

		*img = res.img;
		uint8_t padding_size = get_padding((*img).width);

		for (size_t i = 0; i < (*img).height; i++) {
			size_t count = fread((*img).data + (*img).width * i, sizeof(struct pixel), (*img).width, in);
			if (count != (*img).width) {
				fprintf(stderr, "%s", read_status_string[READ_INVALID_BITS]);
				return READ_INVALID_BITS;
			}
			if (fseek(in, padding_size, SEEK_CUR) != 0) {
				fprintf(stderr, "%s", read_status_string[FSEEK_ERROR]);
				return FSEEK_ERROR;
			}
		}
		return READ_OK;
	}
	return ALLOCATION_ERROR;

}

enum write_status to_bmp(FILE* const out, struct image const* img) {

	uint8_t padding_size = get_padding((*img).width);
	uint64_t picture_size = (*img).height * ((*img).width * sizeof(struct pixel) + padding_size);

	struct bmp_header cur_header = {
		.bfType = BF_TYPE,
		.bfileSize = (uint32_t) picture_size + sizeof(struct bmp_header),
		.bfReserved = BF_RESERVED,
		.bOffBits = B_OFF_BITS,
		.biSize = BI_SIZE,
		.biWidth = (uint32_t) (*img).width,
		.biHeight = (uint32_t) (*img).height,
		.biPlanes = BI_PLANES,
		.biBitCount = BI_BIT_COUNT,
		.biCompression = BI_COMPRESSION,
		.biSizeImage = (uint32_t) picture_size,
		.biXPelsPerMeter = BI_PELS_PER_METER,
		.biYPelsPerMeter = BI_PELS_PER_METER,
		.biClrUsed = BI_CLR_USED,
		.biClrImportant = BI_CLR_IMPORTANT
	};
	if (fwrite(&cur_header, sizeof(struct bmp_header), 1, out) != 1) {
		fprintf(stderr, "%s", read_status_string[WRITE_HEADER_ERROR]);
		return WRITE_HEADER_ERROR;
	}
	for (size_t i = 0; i < (*img).height; i++) {
		size_t count = fwrite((*img).data + i * (*img).width, sizeof(struct pixel), (*img).width, out);
		if (count != (*img).width) {
			fprintf(stderr, "%s", read_status_string[WRITE_PIXELS_ERROR]);
			return WRITE_PIXELS_ERROR;
		}
		if (fwrite("\0\0\0", padding_size, 1, out) != 1) {
			fprintf(stderr, "%s", read_status_string[WRITE_PADDING_ERROR]);
			return WRITE_PADDING_ERROR;
		}
	}
	return WRITE_OK;
}
