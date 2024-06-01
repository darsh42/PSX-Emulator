CC=gcc
FILES=src/core/*.c src/sdl/*.c src/gui/*.c
DEBUG=src/debug/*.c
FLAGS=
FLAGS_DEBUG=-g -fsanitize=address -Wall -Wextra
LIBS=-lm -lSDL2 
TARGET=target/psx

debug:
	$(CC) $(FILES) $(DEBUG) -o $(TARGET) $(FLAGS) $(FLAGS_DEBUG) $(LIBS)

psx:
	$(CC) $(FILES) -o $(TARGET) $(FLAGS) $(LIBRARIES)
