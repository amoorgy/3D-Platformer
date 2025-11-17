# Simple Makefile for the 3D Collectibles Game
# Adjust for your platform as needed

# Compiler
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

# Libraries (Linux/macos)
# macOS usually uses frameworks
LIBS = -framework OpenGL -framework GLUT -framework AudioToolbox -framework AudioUnit -framework CoreAudio -lpthread

# If on Linux, you might need:
# LIBS = -lGL -lGLU -lglut -lpthread -ldl

# Target executable
TARGET = PXX_YYYY

# Source files
SOURCES = PXX_YYYY.cpp

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: clean