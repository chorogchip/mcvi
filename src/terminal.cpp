#include "terminal.hpp"

#include <cstdlib>
#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>

namespace mcvi {

void write_all(std::string_view data) {
    while (!data.empty()) {
        ssize_t written = write(STDOUT_FILENO, data.data(), data.size());
        if (written <= 0) {
            return;
        }
        data.remove_prefix(static_cast<std::size_t>(written));
    }
}

TerminalSize terminal_size() {
    winsize ws{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return {};
    }
    return {static_cast<int>(ws.ws_row), static_cast<int>(ws.ws_col)};
}

char read_key() {
    char ch = 0;
    ssize_t nread = read(STDIN_FILENO, &ch, 1);
    if (nread == 1) {
        return ch;
    }
    return 0;
}

RawTerminal::RawTerminal() {
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) {
        std::cerr << "mcvi needs an interactive terminal.\n";
        std::exit(1);
    }
    if (tcgetattr(STDIN_FILENO, &original_) == -1) {
        std::perror("tcgetattr");
        std::exit(1);
    }

    termios raw = original_;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= CS8;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        std::perror("tcsetattr");
        std::exit(1);
    }

    write_all("\x1b[?25l");
}

RawTerminal::~RawTerminal() {
    write_all("\x1b[?25h\x1b[0m\x1b[2J\x1b[H");
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_);
}

} // namespace mcvi
