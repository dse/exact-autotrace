#include <stdio.h>
#include <stdlib.h>

#include "../bitmap/bmp.h"

#define BM_BPP 4
extern int exact_autotrace_width;
extern int exact_autotrace_height;
extern int exact_autotrace_verbose;

FILE *fp;
Bitmap *bmp;

unsigned char exact_autotrace_pixel_value(int x, int y) {
    unsigned char r = bmp->data[y * exact_autotrace_width * BM_BPP + x * BM_BPP + 2];
    unsigned char g = bmp->data[y * exact_autotrace_width * BM_BPP + x * BM_BPP + 1];
    unsigned char b = bmp->data[y * exact_autotrace_width * BM_BPP + x * BM_BPP];
    int l = ((2126 * r + 7152 * g + 722 * b) + 5000) / 10000;
    if (l < 0) {
        l = 0;
    } else if (l > 255) {
        l = 255;
    }
    return (unsigned char)l;
}

void exact_autotrace_init(char *filename) {
    if (!(fp = fopen(filename, "rb"))) {
        perror("fopen");
        exit(1);
    }
    if (!(bmp = bm_load_fp(fp))) {
        fprintf(stderr, "bm_load_fp error\n");
        exit(1);
    }
    exact_autotrace_width  = bmp->w;
    exact_autotrace_height = bmp->h;
}

void exact_autotrace_cleanup() {
}
