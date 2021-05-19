/**
 * exact-autotrace.c
 *
 * Copyright (c) 2020 Darren Embry
 *
 * GPL2.
 */

#include <stdio.h>
#include <stdlib.h>

/* platform dependent */
void exact_autotrace_init(char *);
void exact_autotrace_cleanup();
unsigned char exact_autotrace_pixel_value(int x, int y);

/* independent */
void exact_autotrace(char *);
void exact_autotrace_output_start();
void exact_autotrace_output_pixel(int x, int y);
void exact_autotrace_output_end();

int exact_autotrace_verbose = 0;

int main(int argc, char **argv) {
    if (getenv("EXACT_AUTOTRACE_VERBOSE")) {
        exact_autotrace_verbose = atoi(getenv("EXACT_AUTOTRACE_VERBOSE"));
    }
    argc -= 1;
    argv += 1;
    if (argc < 1) {
        fprintf(stderr, "no filename argument\n");
        exit(1);
    }
    if (exact_autotrace_verbose) {
        fprintf(stderr, "exact-autotrace:");
        for (int i = 0; i < argc; i += 1) {
            fprintf(stderr, " %s", argv[i]);
        }
        fprintf(stderr, "\n");
    }
    char *filename = argv[argc - 1];
    if (exact_autotrace_verbose) {
        fprintf(stderr, "exact-autotrace: filename = %s\n", filename);
    }
    exact_autotrace(filename);
    if (exact_autotrace_verbose) {
        fprintf(stderr, "exact-autotrace: done\n"); fflush(stderr);
    }
}

int exact_autotrace_width = 0;
int exact_autotrace_height = 0;

void exact_autotrace_output_start() {
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

void exact_autotrace_output_horizontal_pixel_block(int x, int x2, int y) {
    printf("%d %d m\n", x, y);
    printf("%d %d l\n", x, y + 1);
    printf("%d %d l\n", x2 + 1, y + 1);
    printf("%d %d l\n", x2 + 1, y);
    printf("%d %d l\n", x, y);
}

void exact_autotrace_output_end() {
    puts("*U");
    puts("%%Trailer");
    puts("%%EOF");
}

int lavg;

void exact_autotrace_collect() {
    int l, lmax, lmin, y, x;
    for (y = 0; y < exact_autotrace_height; y += 1) {
        for (x = 0; x < exact_autotrace_width; x += 1) {
            l = exact_autotrace_pixel_value(x, y);
            if (x == 0 && y == 0) {
                lmin = lmax = l;
            } else {
                if (lmin > l) { lmin = l; }
                if (lmax < l) { lmax = l; }
            }
        }
    }
    lavg = (lmin + lmax) / 2;
}

void exact_autotrace_run_brute_force() {
    exact_autotrace_output_start();
    int y, x, l;
    for (y = 0; y < exact_autotrace_height; y += 1) {
        for (x = 0; x < exact_autotrace_width; x += 1) {
            l = exact_autotrace_pixel_value(x, y);
            if (l < lavg) {
                exact_autotrace_output_pixel(x, exact_autotrace_height - 1 - y);
            }
        }
    }
    exact_autotrace_output_end();
}

void exact_autotrace_run_2() {
    exact_autotrace_output_start();
    int y, x, x2, l;
    for (y = 0; y < exact_autotrace_height; y += 1) {
        for (x = 0; x < exact_autotrace_width; x += 1) {
            l = exact_autotrace_pixel_value(x, y);
            if (l < lavg) {
                for (x2 = x + 1;
                     x2 < exact_autotrace_width &&
                         (l = exact_autotrace_pixel_value(x2, y)) < lavg;
                     x2 += 1) {
                }
                x2 -= 1;
                exact_autotrace_output_horizontal_pixel_block(x, x2, exact_autotrace_height - 1 - y);
                x = x2 + 1;
            }
        }
    }
    exact_autotrace_output_end();
}

void exact_autotrace(char *filename) {
    exact_autotrace_init(filename);
    if (exact_autotrace_verbose) {
        fprintf(stderr, "exact-autotrace: %s, %d x %d\n", filename, exact_autotrace_width, exact_autotrace_height);
    }
    exact_autotrace_collect();
    exact_autotrace_run_2();
    exact_autotrace_cleanup();
    if (exact_autotrace_verbose) {
        fprintf(stderr, "exact-autotrace: done\n");
    }
}
