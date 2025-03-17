CC := gcc

HEADERS := $(wildcard *.h)
SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))
TARGET  := psx

LIBRARY := -lpthread -lSDL2

CFLAGS := -g -Wall -Wextra -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wstrict-overflow=5 -pthread # -fsanitize=address  -DDEBUG -Wpedantic 

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBRARY) -I.

%.o: %.c trace.h
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBRARY) -I.

.PHONY: clean run debug stub run_debug all

BIOS := scph1001.bin
GAME := .
CPU_TEST := psxtest_cpu.exe

clean:
	rm -rf $(TARGET) $(OBJECTS)

run: $(TARGET)
	./$< -b $(BIOS) -g .$(GAME)

cputest: $(TARGET)
	./$< -b $(BIOS) -e $(CPU_TEST) -g .$(GAME)

debug: $(TARGET)
	gdb -iex "set auto-load safe-path $(shell pwd)" --args ./$< -b $(BIOS) -e $(CPU_TEST) -g .$(GAME)


