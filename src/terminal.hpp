#pragma once

#include <string_view>
#include <termios.h>

namespace mcvi {

struct TerminalSize {
    int rows = 24;
    int cols = 80;
};

void write_all(std::string_view data);
TerminalSize terminal_size();
char read_key();

class RawTerminal {
public:
    RawTerminal();
    RawTerminal(const RawTerminal&) = delete;
    RawTerminal& operator=(const RawTerminal&) = delete;
    ~RawTerminal();

private:
    termios original_{};
};

} // namespace mcvi
