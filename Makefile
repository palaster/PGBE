CC=gcc
CFLAGS=-I/usr/include/SDL2 -D_REENTRANT -g
LDFLAGS=-lSDL2
DEPS = gameboy.h cpu.h mmu.h bit_logic.h
OBJ = gameboy.o cpu.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

gameboy: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)
