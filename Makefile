CC=gcc
CFLAGS=-I/usr/include/SDL2 -D_REENTRANT -g
LDFLAGS=-lSDL2
DEPS = gameboy.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

gameboy: gameboy.o
	$(CC) -o gameboy gameboy.o ${LDFLAGS}
