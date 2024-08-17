CC=gcc
FILES=src/core/*.c src/renderer/*.c
DEBUG=src/debug/*.c
TARGET=target/psx
INCLUDES=./include

WARNINGS=-Wall -Wextra 
IGNORE_WARNINGS=-Wno-type-limits -Wno-unused-function -Wno-sign-compare -Wno-unused-parameter
LIBRARIES=-lm -lSDL2 -lreadline
FLAGS=$(WARNINGS) $(IGNORE_WARNINGS)
DEBUGFLAGS=-g -pg -fsanitize=address 

run-debug:
	clear 
	make debug
	./target/psx misc/SCPH1001.BIN .

debug:
	$(CC) $(FILES) $(DEBUG) -o $(TARGET) -I$(INCLUDES) $(FLAGS) $(DEBUGFLAGS) $(LIBRARIES)

psx:
	$(CC) $(FILES) -o $(TARGET) -I$(INCLUDES) $(FLAGS) $(LIBRARIES) -O3
