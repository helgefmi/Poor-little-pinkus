CFLAGS=-O3 -Wall -Wextra
SRC=src/cache.o src/main.o src/move.o src/state.o src/test.o src/util.o
TARGET=main

all: $(SRC)
	$(CC) -o $(TARGET) $(SRC) $(CFLAGS)

clean:
	rm -f src/*.o
