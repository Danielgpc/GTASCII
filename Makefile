# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17

# Automatically detect Homebrew path for Apple Silicon or Intel Macs
ifeq ($(shell uname -m), arm64)
    BREW_PREFIX = /opt/homebrew
else
    BREW_PREFIX = /usr/local
endif

# Include paths and libraries for ncurses
# macOS uses ncurses instead of curses
INC = -I$(BREW_PREFIX)/opt/ncurses/include
LIB = -L$(BREW_PREFIX)/opt/ncurses/lib -lncurses

# Target executable name
TARGET = bin/app

# Source files
SRC = src/main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir bin
	$(CXX) $(CXXFLAGS) $(INC) $(SRC) -o $(TARGET) $(LIB)

clean:
	rm -rf bin

run: all
	./$(TARGET)

.PHONY: all clean run











































