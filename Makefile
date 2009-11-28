CFLAGS=-O3 -g2 -Wall -Wextra
SRC=src/bench.o src/next.o src/sort.o src/search.o src/timectrl.o src/main.o src/move.o src/util.o src/hash.o src/uci.o src/eval.o src/state.o src/test.o src/cache.o
TARGET=plp

all: $(SRC)
	$(CC) -o $(TARGET) $(SRC) $(CFLAGS)

clean:
	rm -f src/*.o $(TARGET)
