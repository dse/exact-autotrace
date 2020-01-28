SRC = src/exact-autotrace.c
BIN = bin/exact-autotrace-c

$(BIN): $(SRC)
#	cc -o $(BIN) $(SRC) `pkg-config --cflags --libs Wand`
	cc -o $(BIN) $(SRC) libbmp/libbmp.c
