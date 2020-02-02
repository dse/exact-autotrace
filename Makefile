SRC = src/exact-autotrace.c src/exact-autotrace-bitmap-diy.c bitmap/bmp.c
BIN = bin/exact-autotrace-c

CFLAGS = -c -Wall -Wextra

$(BIN): $(SRC) Makefile
	cc -o $(BIN) $(SRC)
