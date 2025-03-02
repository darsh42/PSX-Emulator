CC := gcc

HEADERS := $(wildcard *.h)
SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))
TARGET  := psx

LIBRARY := -lpthread -lSDL2

CFLAGS := -g -Wall -Wextra -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wstrict-overflow=5 -pthread -fsanitize=address # -DDEBUG -Wpedantic 

$(TARGET): $(OBJECTS) trace.h
	$(CC) $(CFLAGS) $^ -o $@ $(LIBRARY) -I.

%.o: %.c; 
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBRARY) -I.

.PHONY: clean run debug stub run_debug all

BIOS := scph1001.bin
GAME := .

clean:
	rm -rf $(TARGET) $(OBJECTS)

run: $(TARGET)
	./$< -b $(BIOS) -g .$(GAME)

debug: $(TARGET)
	gdb -iex "set auto-load safe-path $(shell pwd)" --args ./$< -b $(BIOS) -g .$(GAME)


