CXX ?= g++
CXXFLAGS ?= -O2 -Wall -Wextra -std=c++17
SDL_CFLAGS := $(shell pkg-config --cflags sdl2 2>/dev/null)
SDL_LIBS := $(shell pkg-config --libs sdl2 2>/dev/null || echo "-lSDL2")

TARGET = ygo_calc
SRC = src/main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) $(SRC) -o $(TARGET) $(SDL_LIBS)

bundle-libs: $(TARGET)
	mkdir -p lib
	ldd $(TARGET) | awk '/=> \// {print $$3}' | grep -E 'libSDL2' | xargs -r -I{} cp -L {} lib/

clean:
	rm -f $(TARGET)
