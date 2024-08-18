CC	:=gcc

_DIR_INC     := include
_DIR_SRC     := src
_DIR_MODULES := core debug renderer
_DIR_BUILD   := build

INCLUDE := -I$(_DIR_INC)
SOURCES := $(foreach module,$(_DIR_MODULES),$(wildcard $(_DIR_SRC)/$(module)/*.c))
OBJECTS := $(patsubst $(_DIR_SRC)/%.c,$(_DIR_BUILD)/%.o,$(SOURCES))
TARGET  := $(_DIR_BUILD)/psx

WARNINGS        := -Wall -Wextra 
IGNORE_WARNINGS := -Wno-type-limits -Wno-unused-function -Wno-sign-compare -Wno-unused-parameter
LIBRARIES       := -lm -lSDL2 -lreadline # -lubsan
DEBUGFLAGS      := -g #-pg -fsanitize=undefined
CFLAGS          := -O3 $(WARNINGS) $(IGNORE_WARNINGS) $(INCLUDE) # $(DEBUGFLAGS)

$(shell mkdir -p $(addprefix $(_DIR_BUILD)/, $(_DIR_MODULES)))

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBRARIES)

$(_DIR_BUILD)/%.o: $(_DIR_SRC)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean run debug

run:
	./$(TARGET) misc/SCPH1001.BIN .

debug:
	gdb --args ./$(TARGET) misc/SCPH1001.BIN .

clean:
	rm -rf $(_DIR_BUILD)
