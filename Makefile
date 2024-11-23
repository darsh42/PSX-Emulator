CC := gcc

SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))
TARGET  := psx

LIBRARY := -lpthread -lSDL2

CFLAGS := -g -Wall -Wextra -Wpedantic -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wstrict-overflow=5 -pthread # -DDEBUG

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBRARY) 

%.o: %.c; 
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBRARY) 

.PHONY: clean run debug stub run_debug all

BIOS := SCPH1001.BIN
GAME := .

clean:
	rm -rf $(TARGET) $(OBJECTS)

run: $(TARGET)
	./$< -b $(BIOS) -g .$(GAME)

debug: $(TARGET)
	gdb -iex "set auto-load safe-path $(shell pwd)" --args ./$< -b $(BIOS) -g .$(GAME)


