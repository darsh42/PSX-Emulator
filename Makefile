CC := gcc

# Directories
_DIR_INC     := include
_DIR_SRC     := src
_DIR_MODULES := core debug renderer
_DIR_BUILD   := build

# Files
INCLUDE := -I$(_DIR_INC)
SOURCES := $(foreach module,$(_DIR_MODULES),$(wildcard $(_DIR_SRC)/$(module)/*.c))
OBJECTS := $(patsubst $(_DIR_SRC)/%.c,$(_DIR_BUILD)/%.o,$(SOURCES))
TARGET  := $(_DIR_BUILD)/psx

# compiler options and libraries
WARNINGS        := -Wall -Wextra 
IGNORE_WARNINGS := -Wno-type-limits -Wno-unused-function -Wno-sign-compare -Wno-unused-parameter
LIBRARIES       := -lm -lSDL2 -lreadline # -lubsan
DEBUGFLAGS      := -g #-pg -fsanitize=undefined
CFLAGS          := $(WARNINGS) $(IGNORE_WARNINGS) $(INCLUDE) $(DEBUGFLAGS) # -O3 

# create build directories
$(shell mkdir -p $(addprefix $(_DIR_BUILD)/, $(_DIR_MODULES)))

# link to target
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBRARIES)

# compile files to objects
$(_DIR_BUILD)/%.o: $(_DIR_SRC)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean run debug

# run based on default structure
run:
	./$(TARGET) misc/SCPH1001.BIN .

# run debug based on default structure
debug:
	gdb --args ./$(TARGET) misc/SCPH1001.BIN .

# delete all build directories
clean:
	rm -rf $(_DIR_BUILD)
