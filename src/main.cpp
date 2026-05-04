#include <algorithm>
#include <cctype>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unordered_map>
#include <unistd.h>

namespace {

constexpr char ESC = 27;
constexpr char CTRL_C = 3;
constexpr char BACKSPACE = 127;
constexpr char ENTER = '\r';

volatile std::sig_atomic_t g_interrupted = 0;

void write_all(std::string_view data) {
    while (!data.empty()) {
        ssize_t written = write(STDOUT_FILENO, data.data(), data.size());
        if (written <= 0) {
            return;
        }
        data.remove_prefix(static_cast<size_t>(written));
    }
}

void on_signal(int) {
    g_interrupted = 1;
}

struct TerminalSize {
    int rows = 24;
    int cols = 80;
};

TerminalSize terminal_size() {
    winsize ws{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return {};
    }
    return {static_cast<int>(ws.ws_row), static_cast<int>(ws.ws_col)};
}

class RawTerminal {
public:
    RawTerminal() {
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

    RawTerminal(const RawTerminal&) = delete;
    RawTerminal& operator=(const RawTerminal&) = delete;

    ~RawTerminal() {
        write_all("\x1b[?25h\x1b[0m\x1b[2J\x1b[H");
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_);
    }

private:
    termios original_{};
};

enum class Mode {
    Normal,
    Insert,
    Command,
};

const char* mode_name(Mode mode) {
    switch (mode) {
    case Mode::Normal:
        return "NORMAL";
    case Mode::Insert:
        return "INSERT";
    case Mode::Command:
        return "COMMAND";
    }
    return "";
}

struct Pos {
    int x = 0;
    int y = 0;
    int z = 0;

    friend bool operator==(const Pos&, const Pos&) = default;
};

struct PosHash {
    std::size_t operator()(const Pos& pos) const {
        std::size_t h = static_cast<std::size_t>(pos.x);
        h = h * 1315423911u + static_cast<std::size_t>(pos.y);
        h = h * 1315423911u + static_cast<std::size_t>(pos.z);
        return h;
    }
};

class World {
public:
    char block_at(Pos pos) const {
        auto it = blocks_.find(pos);
        if (it == blocks_.end()) {
            return ' ';
        }
        return it->second;
    }

    void set_block(Pos pos, char block) {
        if (block == ' ') {
            blocks_.erase(pos);
            return;
        }
        blocks_[pos] = block;
    }

    int max_x_on_row(int y, int z) const {
        int max_x = 0;
        for (const auto& [pos, block] : blocks_) {
            if (block != ' ' && pos.y == y && pos.z == z) {
                max_x = std::max(max_x, pos.x);
            }
        }
        return max_x;
    }

private:
    std::unordered_map<Pos, char, PosHash> blocks_;
};

struct Editor {
    Mode mode = Mode::Normal;
    World world;
    Pos cursor;
    std::string command;
    std::string message = "mcvi slice z=0";
    bool running = true;
};

int clamp_int(int value, int low, int high) {
    return std::max(low, std::min(value, high));
}

void move_left(Editor& editor) {
    if (editor.cursor.x > 0) {
        --editor.cursor.x;
    }
}

void move_right(Editor& editor) {
    ++editor.cursor.x;
}

void move_up(Editor& editor) {
    if (editor.cursor.y > 0) {
        --editor.cursor.y;
    }
}

void move_down(Editor& editor) {
    ++editor.cursor.y;
}

void insert_char(Editor& editor, char ch) {
    editor.world.set_block(editor.cursor, ch);
    ++editor.cursor.x;
}

void insert_newline(Editor& editor) {
    ++editor.cursor.y;
    editor.cursor.x = 0;
}

void backspace(Editor& editor) {
    if (editor.cursor.x > 0) {
        --editor.cursor.x;
        editor.world.set_block(editor.cursor, ' ');
    }
}

void execute_command(Editor& editor) {
    if (editor.command == "q") {
        editor.running = false;
        return;
    }

    editor.message = "Not an editor command: " + editor.command;
    editor.command.clear();
    editor.mode = Mode::Normal;
}

void handle_normal(Editor& editor, char ch) {
    switch (ch) {
    case 'h':
        move_left(editor);
        break;
    case 'j':
        move_down(editor);
        break;
    case 'k':
        move_up(editor);
        break;
    case 'l':
        move_right(editor);
        break;
    case '0':
        editor.cursor.x = 0;
        break;
    case '$':
        editor.cursor.x = editor.world.max_x_on_row(editor.cursor.y, editor.cursor.z);
        break;
    case 'i':
        editor.mode = Mode::Insert;
        break;
    case 'a':
        move_right(editor);
        editor.mode = Mode::Insert;
        break;
    case ':':
        editor.command.clear();
        editor.mode = Mode::Command;
        break;
    default:
        break;
    }
}

void handle_insert(Editor& editor, char ch) {
    if (ch == ESC) {
        editor.mode = Mode::Normal;
        return;
    }
    if (ch == ENTER || ch == '\n') {
        insert_newline(editor);
        return;
    }
    if (ch == BACKSPACE || ch == '\b') {
        backspace(editor);
        return;
    }
    if (std::isprint(static_cast<unsigned char>(ch))) {
        insert_char(editor, ch);
    }
}

void handle_command(Editor& editor, char ch) {
    if (ch == ESC) {
        editor.command.clear();
        editor.mode = Mode::Normal;
        return;
    }
    if (ch == ENTER || ch == '\n') {
        execute_command(editor);
        return;
    }
    if (ch == BACKSPACE || ch == '\b') {
        if (!editor.command.empty()) {
            editor.command.pop_back();
        }
        return;
    }
    if (std::isprint(static_cast<unsigned char>(ch))) {
        editor.command.push_back(ch);
    }
}

void handle_key(Editor& editor, char ch) {
    if (ch == CTRL_C) {
        editor.running = false;
        return;
    }

    switch (editor.mode) {
    case Mode::Normal:
        handle_normal(editor, ch);
        break;
    case Mode::Insert:
        handle_insert(editor, ch);
        break;
    case Mode::Command:
        handle_command(editor, ch);
        break;
    }
}

std::string crop_line(std::string_view line, int width) {
    if (width <= 0) {
        return "";
    }
    if (static_cast<int>(line.size()) <= width) {
        return std::string(line);
    }
    return std::string(line.substr(0, static_cast<size_t>(width)));
}

void render(const Editor& editor) {
    TerminalSize size = terminal_size();
    int text_rows = std::max(1, size.rows - 1);
    std::string out;
    out.reserve(static_cast<size_t>(size.rows * size.cols + 128));

    out += "\x1b[?25l\x1b[H";
    for (int row = 0; row < text_rows; ++row) {
        out += "\x1b[K";
        for (int col = 0; col < size.cols; ++col) {
            char block = editor.world.block_at({col, row, editor.cursor.z});
            out.push_back(block);
        }
        out += "\r\n";
    }

    out += "\x1b[7m";
    std::string status = std::string(" ") + mode_name(editor.mode) + "  "
        + "x=" + std::to_string(editor.cursor.x)
        + " y=" + std::to_string(editor.cursor.y)
        + " z=" + std::to_string(editor.cursor.z);
    if (!editor.message.empty() && editor.mode != Mode::Command) {
        status += "  " + editor.message;
    }
    out += crop_line(status, size.cols);
    if (static_cast<int>(status.size()) < size.cols) {
        out.append(static_cast<size_t>(size.cols - status.size()), ' ');
    }
    out += "\x1b[0m";

    if (editor.mode == Mode::Command) {
        out += "\x1b[";
        out += std::to_string(size.rows);
        out += ";1H\x1b[K:";
        out += crop_line(editor.command, size.cols - 1);
    }

    int screen_row = clamp_int(editor.cursor.y + 1, 1, text_rows);
    int screen_col = clamp_int(editor.cursor.x + 1, 1, size.cols);
    if (editor.mode == Mode::Command) {
        screen_row = size.rows;
        screen_col = clamp_int(static_cast<int>(editor.command.size()) + 2, 1, size.cols);
    }
    out += "\x1b[";
    out += std::to_string(screen_row);
    out += ";";
    out += std::to_string(screen_col);
    out += "H\x1b[?25h";

    write_all(out);
}

char read_key() {
    char ch = 0;
    ssize_t nread = read(STDIN_FILENO, &ch, 1);
    if (nread == 1) {
        return ch;
    }
    return 0;
}

} // namespace

int main() {
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    RawTerminal terminal;
    Editor editor;

    while (editor.running && !g_interrupted) {
        render(editor);
        char ch = read_key();
        if (ch != 0) {
            handle_key(editor, ch);
        }
    }

    return 0;
}
