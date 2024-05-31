CC=gcc
FILES=src/core/*.c src/sdl/*.c src/gui/*.c
DEBUG=src/debug/*.c
FLAGS=-g -fsanitize=address -Wall -Wextra -lm -lSDL2 
TARGET=target/psx

psx_debug:
	$(CC) $(FILES) $(DEBUG) -o $(TARGET) $(FLAGS) $(LIBRARIES)

psx:
	$(CC) $(FILES) -o $(TARGET) $(FLAGS) $(LIBRARIES)
