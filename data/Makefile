# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -I./src
LDFLAGS = 

# Project name
TARGET = banking_system

# Source files
SRCDIR = src
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Create executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create data directory
data:
	mkdir -p data

# Run the program
run: $(TARGET) data
	./$(TARGET)

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Clean everything including data
distclean: clean
	rm -rf data

# Install (create necessary directories)
install: data

.PHONY: all run clean distclean install