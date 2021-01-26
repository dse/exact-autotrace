#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int exact_autotrace_width;
extern int exact_autotrace_height;

struct bmpfile_magic {
    unsigned char magic[2];     /* "BM" */
} __attribute__((__packed__));

struct bmpfile_header {
    uint32_t filesz;            /* size of file in bytes */
    uint16_t creator1;
    uint16_t creator2;
    uint32_t bmp_offset;        /* starting address of pixel data */
} __attribute__((__packed__));

struct bmpfile_dibinfo {        /* BITMAPINFOHEADER */
    uint32_t header_sz;         /* size of THIS header */
    int32_t width;              /* pixel width */
    int32_t height;             /* pixel heigh */
    uint16_t nplanes;           /* number of color planes, must be 1 */
    uint16_t bitspp;            /* number of bits per pixel, we only recognize 1 */
    uint32_t compress_type;     /* we only recognize 0 */
    uint32_t bmp_bytesz;        /* size of raw bitmap data, or 0 */
    int32_t hres;
    int32_t vres;
    uint32_t ncolors;           /* # colors in palette, or 0 */
    uint32_t nimpcolors;
} __attribute__((__packed__));

struct bmpfile_colinfo {
    uint8_t b, g, r, a;
} __attribute__((__packed__));

static char *data = NULL;
static size_t rowsize = 0;
static size_t datasize = 0;
static struct bmpfile_magic magic;
static struct bmpfile_header header;
static struct bmpfile_dibinfo dibinfo;
static struct bmpfile_colinfo *palette = NULL;

unsigned char exact_autotrace_pixel_value(int x, int y) {
    size_t offset = rowsize * (dibinfo.height - 1 - y) + (x >> 3);
    uint8_t bit = (data[offset] >> (7 - x % 8)) & 1;
    uint8_t r = palette[bit].r;
    uint8_t g = palette[bit].g;
    uint8_t b = palette[bit].b;
    int l = ((2126 * r + 7152 * g + 722 * b) + 5000) / 10000;
    if (l < 0) {
        l = 0;
    } else if (l > 255) {
        l = 255;
    }
    return (unsigned char)l;
}

void exact_autotrace_init(char *filename) {
    FILE *fp;
    if (!(fp = fopen(filename, "rb"))) {
        perror("fopen");
        exit(1);
    }
    if (fread(&magic, sizeof magic, 1, fp) < 1) {
        perror("fread on magic");
        exit(1);
    }
    if (memcmp(magic.magic, "BM", 2)) {
        fprintf(stderr, "bad magic\n");
        exit(1);
    }
    if (fread(&header, sizeof header, 1, fp) < 1) {
        perror("fread on header");
        exit(1);
    }
    if (fread(&dibinfo, sizeof dibinfo, 1, fp) < 1) {
        perror("fread on dibinfo");
        exit(1);
    }
    if (dibinfo.bitspp != 1) {
        fprintf(stderr, "unsupported BMP type\n");
        exit(1);
    }
    if (dibinfo.compress_type != 0) {
        fprintf(stderr, "unsupported compression type\n");
        exit(1);
    }
    exact_autotrace_width  = dibinfo.width;
    exact_autotrace_height = dibinfo.height;
    if (!dibinfo.ncolors) {
        dibinfo.ncolors = 1 << dibinfo.bitspp;
    }
    if (!(palette = calloc(dibinfo.ncolors, sizeof(struct bmpfile_colinfo)))) {
        perror("calloc");
        exit(1);
    }
    if (fread(palette, sizeof(struct bmpfile_colinfo), dibinfo.ncolors, fp) < dibinfo.ncolors) {
        perror("fread on palette");
        exit(1);
    }
    if (fseek(fp, header.bmp_offset, SEEK_SET) != 0) {
        perror("fseek");
        exit(1);
    }
    rowsize = ((((dibinfo.width * dibinfo.bitspp + 7) / 8) + 3) / 4) * 4;
    datasize = rowsize * dibinfo.height;
    if ((data = malloc(datasize)) == NULL) {
        perror("malloc");
        exit(1);
    }
    if (dibinfo.bmp_bytesz) {
        if (dibinfo.bmp_bytesz < datasize) {
            fprintf(stderr, "BMP info specifies data size (%u) less than should be (%zu)\n", dibinfo.bmp_bytesz, datasize);
            exit(1);
        }
        if (dibinfo.bmp_bytesz > datasize) {
            fprintf(stderr, "BMP info specifies data size (%u) greater than should be (%zu)\n", dibinfo.bmp_bytesz, datasize);
            exit(1);
        }
    }
    if (fread(data, datasize, 1, fp) < 1) {
        perror("fread on data");
        exit(1);
    }
}

void exact_autotrace_cleanup() {
    if (data != NULL) {
        free(data);
        data = NULL;
    }
    if (palette != NULL) {
        free(palette);
        palette = NULL;
    }
    exact_autotrace_width = 0;
    exact_autotrace_height = 0;
    rowsize = 0;
    datasize = 0;
}
