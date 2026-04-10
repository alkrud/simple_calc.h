CC=gcc
SRC=$(wildcard ./src/*.c)
HEADERS=$(wildcard ./src/*.h)
CFLAGS=-O3 -Werror -Wall -Wextra -pedantic
DFLAGS=-O0 -g -Wall -Wextra
LFLAGS=-lm
BUILD=build
EXE=test
DEBUG=test-deb

all: $(EXE) $(DEBUG)

$(EXE): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $(EXE) $(SRC) $(LFLAGS)

$(DEBUG): $(SRC) $(HEADERS)
	$(CC) $(DFLAGS) -o $(DEBUG) $(SRC) $(LFLAGS)

run: $(EXE)
	./$(EXE)

debug: $(DEBUG)
	./$(DEBUG)

.PHONY: clean
clean:
	rm -rf $(EXE) $(DEBUG)
