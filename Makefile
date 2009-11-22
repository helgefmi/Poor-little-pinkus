CFLAGS=-O3 -g2 -Wall -Wextra -fopenmp
SRC=src/cache.o src/main.o src/move.o src/state.o src/test.o src/util.o src/hash.o
TARGET=plp

all: $(SRC)
	$(CC) -o $(TARGET) $(SRC) $(CFLAGS)

clean:
	rm -f src/*.o $(TARGET)
