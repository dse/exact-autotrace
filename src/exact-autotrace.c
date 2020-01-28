/**
 * exact-autotrace.c
 *
 * Copyright (c) 2020 Darren Embry
 *
 * GPL2.
 */

#include <stdio.h>
#include <stdlib.h>

/* #include <wand/magick_wand.h> */
/* #include "../libbmp/libbmp.h" */
#include "../qdbmp/qdbmp.h"

/* prototypes */
#ifdef MAGICKWAND_MAJOR_VERSION
void exact_autotrace_magick(char *);
#endif
#ifdef BMP_MAGIC
void exact_autotrace_libbmp(char *);
#endif
#ifdef QDBMP_VERSION_MAJOR
void exact_autotrace_qdbmp(char *);
#endif
void exact_autotrace_start();
void exact_autotrace_output_pixel(int x, int y);
void exact_autotrace_end();
int main(int, char **);

int main(int argc, char **argv) {
    if (argc < 1) {
        fprintf(stderr, "no filename argument\n");
        exit(1);
    }
    char *filename = argv[argc - 1];

#ifdef MAGICKWAND_MAJOR_VERSION
    exact_autotrace_magick(filename);
#endif
#ifdef BMP_MAGIC
    exact_autotrace_libbmp(filename);
#endif
#ifdef QDBMP_VERSION_MAJOR
    exact_autotrace_qdbmp(filename);
#endif
}

int exact_autotrace_width = 0;
int exact_autotrace_height = 0;

void exact_autotrace_start() {
    puts("%!PS-Adobe-3.0 EPSF-3.0");
    puts("%%Creator: Adobe Illustrator by AutoTrace version 0.31.1");
    puts("%%Title: ");
    puts("%%CreationDate: Thu Aug 28 08:09:29 2014");
    printf("%%BoundingBox: 0 0 %d %d\n", exact_autotrace_width, exact_autotrace_height);
    puts("%%DocumentData: Clean7Bit");
    puts("%%EndComments");
    puts("%%BeginProlog");
    puts("/bd { bind def } bind def");
    puts("/incompound false def");
    puts("/m { moveto } bd");
    puts("/l { lineto } bd");
    puts("/c { curveto } bd");
    puts("/F { incompound not {fill} if } bd");
    puts("/f { closepath F } bd");
    puts("/S { stroke } bd");
    puts("/*u { /incompound true def } bd");
    puts("/*U { /incompound false def f} bd");
    puts("/k { setcmykcolor } bd");
    puts("/K { k } bd");
    puts("%%EndProlog");
    puts("%%BeginSetup");
    puts("%%EndSetup");
    puts("0.000 0.000 0.000 0.498 k");
    puts("*u");
}

void exact_autotrace_output_pixel(int x, int y) {
    printf("%d %d m\n", x, y);
    printf("%d %d l\n", x, y + 1);
    printf("%d %d l\n", x + 1, y + 1);
    printf("%d %d l\n", x + 1, y);
    printf("%d %d l\n", x, y);
}

void exact_autotrace_end() {
    puts("*U");
    puts("%%Trailer");
    puts("%%EOF");
}

#ifdef MAGICKWAND_MAJOR_VERSION
void exact_autotrace_magick(char *filename) {
    MagickWand *mw = NULL;
    PixelWand *pw = NULL;
    MagickWandGenesis();
    mw = NewMagickWand();
    pw = NewPixelWand();

    if (MagickReadImage(mw, filename) == MagickFalse) {
        fprintf(stderr, "read error for %s\n", filename);
        exit(1);
    }

    exact_autotrace_width = (int)MagickGetImageWidth(mw);
    if (exact_autotrace_width < 1) {
        fprintf(stderr, "bad width for %s\n", filename);
        exit(1);
    }

    exact_autotrace_height = (int)MagickGetImageHeight(mw);
    if (exact_autotrace_height < 1) {
        fprintf(stderr, "bad height for %s\n", filename);
        exit(1);
    }

    MagickSetColorspace(mw, GRAYColorspace);
    MagickQuantizeImage(mw, 256, GRAYColorspace, 0, MagickFalse, MagickFalse);

    int x, y;
    double l, lmax, lmin, lavg;

    for (y = 0; y < exact_autotrace_height; y += 1) {
        for (x = 0; x < exact_autotrace_width; x += 1) {
            MagickGetImagePixelColor(mw, x, y, pw);
            l = PixelGetGreen(pw);
            if (x == 0 && y == 0) {
                lmax = l;
                lmin = l;
            } else {
                if (lmin > l) {
                    lmin = l;
                }
                if (lmax < l) {
                    lmax = l;
                }
            }
        }
    }
    lavg = (lmin + lmax) / 2;

    exact_autotrace_start();
    for (y = 0; y < exact_autotrace_height; y += 1) {
        for (x = 0; x < exact_autotrace_width; x += 1) {
            MagickGetImagePixelColor(mw, x, exact_autotrace_height - 1 - y, pw);
            if (PixelGetGreen(pw) < lavg) {
                exact_autotrace_output_pixel(x, y);
            }
        }
    }
    exact_autotrace_end();

    /* Tidy up */
    if (mw) {
        mw = DestroyMagickWand(mw);
    }
    MagickWandTerminus();
}
#endif

#ifdef BMP_MAGIC
void exact_autotrace_libbmp(char *filename) {
    bmp_img img;
    fprintf(stderr, "%s\n", filename);
    enum bmp_error status = bmp_img_read(&img, filename);
    switch (status) {
    case BMP_FILE_NOT_OPENED:
        fprintf(stderr, "BMP file not opened\n");
        exit(1);
    case BMP_HEADER_NOT_INITIALIZED:
        fprintf(stderr, "BMP header not initialized\n");
        exit(1);
    case BMP_INVALID_FILE:
        fprintf(stderr, "invalid BMP file\n");
        exit(1);
    case BMP_ERROR:
        fprintf(stderr, "BMP error\n");
        exit(1);
    }
    exact_autotrace_width = img.img_header.biWidth;
    exact_autotrace_height = img.img_header.biHeight;
    int r, g, b, l, lmax, lmin, lavg, y, x;
    for (y = 0; y < exact_autotrace_height; y += 1) {
        for (x = 0; x < exact_autotrace_width; x += 1) {
            r = img.img_pixels[y][x].red;
            g = img.img_pixels[y][x].green;
            b = img.img_pixels[y][x].blue;
            l = ((r * 2126 + g * 7152 + b * 722) + 5000) / 10000;
            if (x == 0 && y == 0) {
                lmin = lmax = l;
            } else {
                if (lmin > l) { lmin = l; }
                if (lmax < l) { lmax = l; }
            }
        }
    }
    lavg = (lmin + lmax) / 2;
    exact_autotrace_start();
    for (y = 0; y < exact_autotrace_height; y += 1) {
        for (x = 0; x < exact_autotrace_width; x += 1) {
            r = img.img_pixels[y][x].red;
            g = img.img_pixels[y][x].green;
            b = img.img_pixels[y][x].blue;
            l = ((r * 2126 + g * 7152 + b * 722) + 5000) / 10000;
            if (l < lavg) {
                exact_autotrace_output_pixel(x, y);
            }
        }
    }
    exact_autotrace_end();
}
#endif

#ifdef QDBMP_VERSION_MAJOR
void exact_autotrace_qdbmp(char *filename) {
    BMP* bmp;
    fprintf(stderr, "A\n");
    bmp = BMP_ReadFile(filename);
    fprintf(stderr, "B\n");
    if (BMP_GetError() != BMP_OK) {
        fprintf(stderr,  "BMP_ReadFile: An error has occurred: %s (code %d)\n", BMP_GetErrorDescription(), BMP_GetError());
        exit(1);
    }
    exact_autotrace_width  = (int)BMP_GetWidth(bmp);
    if (BMP_GetError() != BMP_OK) {
        fprintf(stderr,  "BMP_GetWidth: An error has occurred: %s (code %d)\n", BMP_GetErrorDescription(), BMP_GetError());
        exit(1);
    }
    exact_autotrace_height = (int)BMP_GetHeight(bmp);
    if (BMP_GetError() != BMP_OK) {
        fprintf(stderr,  "BMP_GetHeight: An error has occurred: %s (code %d)\n", BMP_GetErrorDescription(), BMP_GetError());
        exit(1);
    }
    fprintf(stderr, "C\n");
    unsigned char r, g, b;
    int l, lmax, lmin, lavg, y, x;
    for (y = 0; y < exact_autotrace_height; y += 1) {
        for (x = 0; x < exact_autotrace_width; x += 1) {
            BMP_GetPixelRGB(bmp, x, y, &r, &g, &b);
            l = ((2126 * r + 7152 * g + 722 * b) + 5000) / 10000;
            if (x == 0 && y == 0) {
                lmin = lmax = l;
            } else {
                if (lmin > l) { lmin = l; }
                if (lmax < l) { lmax = l; }
            }
        }
    }
    lavg = (lmin + lmax) / 2;
    exact_autotrace_start();
    for (y = 0; y < exact_autotrace_height; y += 1) {
        for (x = 0; x < exact_autotrace_width; x += 1) {
            BMP_GetPixelRGB(bmp, x, y, &r, &g, &b);
            l = ((2126 * r + 7152 * g + 722 * b) + 5000) / 10000;
            if (l < lavg) {
                exact_autotrace_output_pixel(x, y);
            }
        }
    }
    exact_autotrace_end();
}
#endif
