# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17

# Automatically detect Homebrew path for Apple Silicon or Intel Macs[cite: 9]
ifeq ($(shell uname -m), arm64)
    BREW_PREFIX = /opt/homebrew
else
    BREW_PREFIX = /usr/local
endif

# Include paths and libraries for ncurses[cite: 9]
# macOS uses ncurses instead of curses[cite: 9]
INC = -I$(BREW_PREFIX)/opt/ncurses/include
LIB = -L$(BREW_PREFIX)/opt/ncurses/lib -lncurses

# Target executable name[cite: 9]
TARGET = bin/app

# Source and Object files
# Finds all .cc files in src/ and maps them to .o files in bin/
SRC = $(wildcard src/*.cc)
OBJ = $(patsubst src/%.cc, bin/%.o, $(SRC))

all: $(TARGET)

# Link the final executable using the object files
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET) $(LIB)

# Compile each .cc file into a .o file in the bin directory
# The '| bin' part ensures the bin directory exists before compiling
bin/%.o: src/%.cc | bin
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

# Create the bin directory
bin:
	mkdir -p bin

clean:
	rm -rf bin

run: all
	./$(TARGET)

.PHONY: all clean run