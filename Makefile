CC=gcc
FILES=src/core/*.c src/sdl/*.c
DEBUG=src/debug/*.c
FLAGS=-g -pg -fsanitize=address -Wall -Wextra -Wno-unused-parameter -lm -lSDL2 -lreadline
TARGET=target/psx

psx_debug:
	$(CC) $(FILES) $(DEBUG) -o $(TARGET) $(FLAGS) $(LIBRARIES)

psx:
	$(CC) $(FILES) -o $(TARGET) $(FLAGS) $(LIBRARIES)
