# -------------------------
#      Configuration
# -------------------------

CC = emcc
CXX = em++

TARGET = main.js

SRC_DIR = src
INC_DIR = include

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(SOURCES:.cpp=.o)

# Options Emscripten
EM_FLAGS = \
    -s WASM=1 \
    -s FULL_ES3=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
	-s ASSERTIONS=2 \
	--preload-file ./assets/ \
	--preload-file ./shaders/ \
	-s FULL_ES3=1 \
    -O3

CPPFLAGS = -I$(INC_DIR)

# -------------------------
#        Règles
# -------------------------

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(EM_FLAGS)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

run: $(TARGET)
	emrun $(TARGET)

clean:
	rm -f $(SRC_DIR)/*.o $(TARGET)

.PHONY: all clean run
