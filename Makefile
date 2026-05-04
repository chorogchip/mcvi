CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -O2

TARGET := mcvi
SRC := src/main.cpp

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
