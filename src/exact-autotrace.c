/**
 * exact-autotrace.c
 *
 * Copyright (c) 2020 Darren Embry
 *
 * GPL2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define EXACT_AUTOTRACE_MODE_DATA 0
#define EXACT_AUTOTRACE_MODE_EPS  1

/* platform dependent */
void exact_autotrace_init(char *);
void exact_autotrace_cleanup();
unsigned char exact_autotrace_pixel_value(int x, int y);

/* independent */
void exact_autotrace(char *);
void exact_autotrace_output_start();
void exact_autotrace_output_pixel(int x, int y);
void exact_autotrace_output_end();
int exact_autotrace_puts(int mode, const char* s);
int exact_autotrace_printf(int mode, const char* format, ...);
int exact_autotrace_echo_mode(int mode);

int main(int argc, char **argv) {
    if (argc < 1) {
        fprintf(stderr, "no filename argument\n");
        exit(1);
    }
    char *filename = argv[argc - 1];
    exact_autotrace(filename);
}

int exact_autotrace_width = 0;
int exact_autotrace_height = 0;

void exact_autotrace_output_start() {
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "%!PS-Adobe-3.0 EPSF-3.0");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "%%Creator: Adobe Illustrator by AutoTrace version 0.31.1");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "%%Title: ");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "%%CreationDate: Thu Aug 28 08:09:29 2014");
    exact_autotrace_printf(EXACT_AUTOTRACE_MODE_EPS, "%%BoundingBox: 0 0 %d %d\n", exact_autotrace_width, exact_autotrace_height);
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "%%DocumentData: Clean7Bit");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "%%EndComments");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "%%BeginProlog");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "/bd { bind def } bind def");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "/incompound false def");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "/m { moveto } bd");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "/l { lineto } bd");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "/c { curveto } bd");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "/F { incompound not {fill} if } bd");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "/f { closepath F } bd");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "/S { stroke } bd");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "/*u { /incompound true def } bd");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "/*U { /incompound false def f} bd");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "/k { setcmykcolor } bd");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "/K { k } bd");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "%%EndProlog");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "%%BeginSetup");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "%%EndSetup");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "0.000 0.000 0.000 0.498 k");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "*u");
}

void exact_autotrace_output_pixel(int x, int y) {
    exact_autotrace_printf(EXACT_AUTOTRACE_MODE_DATA, "%d %d m\n", x, y);
    exact_autotrace_printf(EXACT_AUTOTRACE_MODE_DATA, "%d %d l\n", x, y + 1);
    exact_autotrace_printf(EXACT_AUTOTRACE_MODE_DATA, "%d %d l\n", x + 1, y + 1);
    exact_autotrace_printf(EXACT_AUTOTRACE_MODE_DATA, "%d %d l\n", x + 1, y);
    exact_autotrace_printf(EXACT_AUTOTRACE_MODE_DATA, "%d %d l\n", x, y);
}

void exact_autotrace_output_horizontal_pixel_block(int x, int x2, int y) {
    exact_autotrace_printf(EXACT_AUTOTRACE_MODE_DATA, "%d %d m\n", x, y);
    exact_autotrace_printf(EXACT_AUTOTRACE_MODE_DATA, "%d %d l\n", x, y + 1);
    exact_autotrace_printf(EXACT_AUTOTRACE_MODE_DATA, "%d %d l\n", x2 + 1, y + 1);
    exact_autotrace_printf(EXACT_AUTOTRACE_MODE_DATA, "%d %d l\n", x2 + 1, y);
    exact_autotrace_printf(EXACT_AUTOTRACE_MODE_DATA, "%d %d l\n", x, y);
}

void exact_autotrace_output_end() {
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "*U");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "%%Trailer");
    exact_autotrace_puts(EXACT_AUTOTRACE_MODE_EPS, "%%EOF");
}

int lavg;

void exact_autotrace_collect() {
    int l, lmax, lmin, y, x;
    for (y = 0; y < exact_autotrace_height; y += 1) {
        if (getenv("EXACT_AUTOTRACE_DEBUG")) {
            fprintf(stderr, "exact_autotrace_collect: y = %4d:", y);
        }
        for (x = 0; x < exact_autotrace_width; x += 1) {
            l = exact_autotrace_pixel_value(x, y);
            if (getenv("EXACT_AUTOTRACE_DEBUG")) {
                fprintf(stderr, " %3d", l);
            }
            if (x == 0 && y == 0) {
                lmin = lmax = l;
            } else {
                if (lmin > l) { lmin = l; }
                if (lmax < l) { lmax = l; }
            }
        }
        if (getenv("EXACT_AUTOTRACE_DEBUG")) {
            putc('\n', stderr);
        }
    }
    /*
     * actual data has black pixels 128, white pixels 255.
     *
     */
    if (lmin == 128 && lmax == 128) {
        /* all black pixels */
        lmin = 128;
        lmax = 255;
    } else if (lmin == 255 && lmax == 255) {
        /* all white pixels */
        lmin = 128;
        lmax = 255;
    }
    lavg = (lmin + lmax) / 2;
    if (getenv("EXACT_AUTOTRACE_DEBUG")) {
        fprintf(stderr, "exact_autotrace_collect: lmin = %d; lmax = %d; lavg = %d\n", lmin, lmax, lavg);
    }
}

/**
 * output each black pixel separately
 */
void exact_autotrace_run_brute_force() {
    exact_autotrace_output_start();
    int y, x, l;
    for (y = 0; y < exact_autotrace_height; y += 1) {
        for (x = 0; x < exact_autotrace_width; x += 1) {
            l = exact_autotrace_pixel_value(x, y);
            if (l < lavg) {
                /* black pixel */
                if (getenv("EXACT_AUTOTRACE_DEBUG")) {
                    fprintf(stderr, "exact_autotrace_run_brute_force: pixel (%d, %d)\n",
                            x, exact_autotrace_height - 1 - y);
                }
                exact_autotrace_output_pixel(x, exact_autotrace_height - 1 - y);
            }
        }
    }
    exact_autotrace_output_end();
}

/**
 * save some space by finding rows of black pixels
 */
void exact_autotrace_run_2() {
    exact_autotrace_output_start();
    int y, x, x2, l;
    for (y = 0; y < exact_autotrace_height; y += 1) {
        for (x = 0; x < exact_autotrace_width; x += 1) {
            l = exact_autotrace_pixel_value(x, y);
            if (l < lavg) {
                /* black pixel */
                for (x2 = x + 1;
                     x2 < exact_autotrace_width &&
                         (l = exact_autotrace_pixel_value(x2, y)) < lavg;
                     x2 += 1) {
                }
                x2 -= 1;
                if (getenv("EXACT_AUTOTRACE_DEBUG")) {
                    fprintf(stderr, "exact_autotrace_run_2: pixel block (%d to %d, %d)\n",
                            x, x2, exact_autotrace_height - 1 - y);
                }
                exact_autotrace_output_horizontal_pixel_block(x, x2, exact_autotrace_height - 1 - y);
                x = x2 + 1;
            }
        }
    }
    exact_autotrace_output_end();
}

void exact_autotrace(char *filename) {
    exact_autotrace_init(filename);
    exact_autotrace_collect();
    exact_autotrace_run_2();
    exact_autotrace_cleanup();
}

int exact_autotrace_puts(int mode, const char* s) {
    int result = puts(s);
    if (exact_autotrace_echo_mode(mode)) {
        fputs(s, stderr);
        fputc('\n', stderr);
    }
    return result;
}

int exact_autotrace_printf(int mode, const char* format, ...) {
    va_list myargs;
    va_start(myargs, format);
    int result = vprintf(format, myargs);
    va_end(myargs);
    va_start(myargs, format);
    if (exact_autotrace_echo_mode(mode)) {
        vfprintf(stderr, format, myargs);
    }
    va_end(myargs);
    return result;
}

int exact_autotrace_echo_mode(int mode) {
    switch (mode) {
    case EXACT_AUTOTRACE_MODE_DATA:
        return (getenv("EXACT_AUTOTRACE_ECHO") || getenv("EXACT_AUTOTRACE_ECHO_DATA")) ? 1 : 0;
    case EXACT_AUTOTRACE_MODE_EPS:
        return getenv("EXACT_AUTOTRACE_ECHO") ? 1 : 0;
    }
    return 0;
}
