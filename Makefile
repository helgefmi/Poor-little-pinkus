CFLAGS=-O3 -g2 -Wall -Wextra
SRC=src/quiescence.o src/next.o src/search.o src/eval.o src/move.o src/make.o src/timectrl.o src/main.o src/util.o src/hash.o src/uci.o src/state.o src/test.o src/cache.o src/bench.o
TARGET=plp

all: $(SRC)
	$(CC) -o $(TARGET) $(SRC) $(CFLAGS)

clean:
	rm -f src/*.o $(TARGET) plp.exe
