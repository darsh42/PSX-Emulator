CC=gcc
FILES=src/core/*.c # src/sdl/*.c
DEBUG=src/debug/*.c
FLAGS=-g
TARGET=target/psx

psx_debug:
	$(CC) $(FILES) $(DEBUG) -o $(TARGET) $(FLAGS)

psx:
	$(CC) $(FILES) -o $(TARGET) $(FLAGS)
