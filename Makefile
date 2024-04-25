CC=gcc
FILES=src/core/*.c # src/sdl/*.c
FLAGS=-g
TARGET=target/psx

psx:
	$(CC) $(FILES) -o $(TARGET) $(FLAGS)
