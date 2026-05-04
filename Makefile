CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -O2

TARGET := mcvi
SRC := src/main.cpp \
	src/world_io.cpp \
	src/world_io_common.cpp \
	src/world_io_json.cpp \
	src/world_io_schem.cpp \
	src/world_io_nbt.cpp \
	src/world_io_mca.cpp
HEADERS := src/pos.hpp \
	src/world.hpp \
	src/world_io.hpp \
	src/world_io_common.hpp \
	src/world_io_formats.hpp

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
