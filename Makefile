CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -O2
LDLIBS := -lz

TARGET := mcvi
SRC := src/main.cpp \
	src/mode.cpp \
	src/block_alias.cpp \
	src/direction.cpp \
	src/editor.cpp \
	src/terminal.cpp \
	src/renderer.cpp \
	src/command.cpp \
	src/input.cpp \
	src/world_io.cpp \
	src/world_io_common.cpp \
	src/world_io_json.cpp \
	src/nbt_writer.cpp \
	src/world_io_schem.cpp \
	src/world_io_nbt.cpp \
	src/world_io_mca.cpp
HEADERS := src/pos.hpp \
	src/world.hpp \
	src/mode.hpp \
	src/block_alias.hpp \
	src/direction.hpp \
	src/editor.hpp \
	src/terminal.hpp \
	src/renderer.hpp \
	src/command.hpp \
	src/input.hpp \
	src/world_io.hpp \
	src/world_io_common.hpp \
	src/world_io_formats.hpp \
	src/nbt_writer.hpp

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC) $(LDLIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
