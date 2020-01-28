SRC = src/exact-autotrace.c src/exact-autotrace-bitmap.c bitmap/bmp.c
BIN = bin/exact-autotrace-c

$(BIN): $(SRC)
	cc -o $(BIN) $(SRC)
