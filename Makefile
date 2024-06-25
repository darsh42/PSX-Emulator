CC=gcc
FILES=src/core/*.c src/renderer/*.c
DEBUG=src/debug/*.c
TARGET=target/psx

WARNINGS=-Wall -Wextra 
IGNORE_WARNINGS=-Wno-type-limits -Wno-unused-function -Wno-sign-compare -Wno-unused-parameter
LIBRARIES=-lm -lSDL2 -lreadline
FLAGS=$(WARNINGS) $(IGNORE_WARNINGS) -g
# -g -pg -fsanitize=address 

run-debug:
	clear 
	make debug
	./target/psx misc/SCPH1001.BIN .

debug:
	$(CC) $(FILES) $(DEBUG) -o $(TARGET) $(FLAGS) $(LIBRARIES)

psx:
	$(CC) $(FILES) -o $(TARGET) $(FLAGS) $(LIBRARIES)
