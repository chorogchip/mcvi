#include <algorithm>
#include <cctype>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <string_view>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "world_io.hpp"

namespace {

using mcvi::Pos;
using mcvi::World;

constexpr char ESC = 27;
constexpr char CTRL_B = 2;
constexpr char CTRL_C = 3;
constexpr char CTRL_D = 4;
constexpr char CTRL_E = 5;
constexpr char CTRL_F = 6;
constexpr char CTRL_U = 21;
constexpr char CTRL_Y = 25;
constexpr char BACKSPACE = 127;
constexpr char ENTER = '\r';

volatile std::sig_atomic_t g_interrupted = 0;

void write_all(std::string_view data) {
    while (!data.empty()) {
        ssize_t written = write(STDOUT_FILENO, data.data(), data.size());
        if (written <= 0) {
            return;
        }
        data.remove_prefix(static_cast<std::size_t>(written));
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

enum class Direction {
    PosX,
    NegX,
    PosY,
    NegY,
    PosZ,
    NegZ,
};

Pos direction_delta(Direction direction) {
    switch (direction) {
    case Direction::PosX:
        return {1, 0, 0};
    case Direction::NegX:
        return {-1, 0, 0};
    case Direction::PosY:
        return {0, 1, 0};
    case Direction::NegY:
        return {0, -1, 0};
    case Direction::PosZ:
        return {0, 0, 1};
    case Direction::NegZ:
        return {0, 0, -1};
    }
    return {1, 0, 0};
}

const char* direction_name(Direction direction) {
    switch (direction) {
    case Direction::PosX:
        return "+x";
    case Direction::NegX:
        return "-x";
    case Direction::PosY:
        return "+y";
    case Direction::NegY:
        return "-y";
    case Direction::PosZ:
        return "+z";
    case Direction::NegZ:
        return "-z";
    }
    return "";
}

struct Editor {
    Mode mode = Mode::Normal;
    World world;
    Pos cursor;
    Direction direction = Direction::PosX;
    std::string command;
    std::string filename;
    std::string message = "mcvi slice y=0";
    bool dirty = false;
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
    if (editor.cursor.z > 0) {
        --editor.cursor.z;
    }
}

void move_down(Editor& editor) {
    ++editor.cursor.z;
}

void move_layer_up(Editor& editor) {
    int top = editor.world.max_y();
    if (editor.cursor.y < top) {
        ++editor.cursor.y;
    }
    editor.message = "layer y=" + std::to_string(editor.cursor.y);
}

void move_layer_down(Editor& editor) {
    int bottom = editor.world.min_y();
    if (editor.cursor.y > bottom) {
        --editor.cursor.y;
    }
    editor.message = "layer y=" + std::to_string(editor.cursor.y);
}

void insert_char(Editor& editor, char ch) {
    editor.world.set_block(editor.cursor, ch);
    editor.dirty = true;
    editor.cursor = editor.cursor + direction_delta(editor.direction);
    editor.cursor.x = std::max(0, editor.cursor.x);
    editor.cursor.z = std::max(0, editor.cursor.z);
}

void insert_newline(Editor& editor) {
    ++editor.cursor.z;
    editor.cursor.x = 0;
}

void backspace(Editor& editor) {
    Pos previous = editor.cursor - direction_delta(editor.direction);
    if (previous.x < 0 || previous.z < 0) {
        return;
    }
    editor.cursor = previous;
    editor.world.set_block(editor.cursor, ' ');
    editor.dirty = true;
}

void set_direction(Editor& editor, Direction direction) {
    editor.direction = direction;
    editor.message = std::string("direction ") + direction_name(direction);
}

std::string trim(std::string_view text) {
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) {
        text.remove_prefix(1);
    }
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) {
        text.remove_suffix(1);
    }
    return std::string(text);
}

std::string command_argument(std::string_view command, std::string_view name) {
    if (!command.starts_with(name)) {
        return "";
    }
    command.remove_prefix(name.size());
    return trim(command);
}

bool write_editor(Editor& editor, std::string filename) {
    if (filename.empty()) {
        filename = editor.filename;
    }
    if (filename.empty()) {
        editor.message = "No file name";
        return false;
    }

    try {
        mcvi::write_world({editor.world, filename, mcvi::format_from_extension(filename)});
    } catch (const std::exception& error) {
        editor.message = error.what();
        return false;
    }

    editor.filename = std::move(filename);
    editor.dirty = false;
    editor.message = "written " + editor.filename;
    return true;
}

void execute_command(Editor& editor) {
    std::string command = trim(editor.command);
    editor.command.clear();
    editor.mode = Mode::Normal;

    if (command == "q") {
        if (editor.dirty) {
            editor.message = "No write since last change";
            return;
        }
        editor.running = false;
        return;
    }
    if (command == "q!") {
        editor.running = false;
        return;
    }
    if (command == "w") {
        (void)write_editor(editor, "");
        return;
    }
    if (command.starts_with("w ")) {
        (void)write_editor(editor, command_argument(command, "w"));
        return;
    }
    if (command == "wq" || command == "x") {
        if (!editor.dirty || write_editor(editor, "")) {
            editor.running = false;
        }
        return;
    }

    editor.message = "Not an editor command: " + command;
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
    case 'H':
        move_layer_up(editor);
        break;
    case 'L':
        move_layer_down(editor);
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
    case CTRL_F:
        set_direction(editor, Direction::PosX);
        break;
    case CTRL_B:
        set_direction(editor, Direction::NegX);
        break;
    case CTRL_U:
        set_direction(editor, Direction::PosY);
        break;
    case CTRL_D:
        set_direction(editor, Direction::NegY);
        break;
    case CTRL_E:
        set_direction(editor, Direction::PosZ);
        break;
    case CTRL_Y:
        set_direction(editor, Direction::NegZ);
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
    return std::string(line.substr(0, static_cast<std::size_t>(width)));
}

void render(const Editor& editor) {
    TerminalSize size = terminal_size();
    int text_rows = std::max(1, size.rows - 1);
    std::string out;
    out.reserve(static_cast<std::size_t>(size.rows * size.cols + 128));

    out += "\x1b[?25l\x1b[H";
    for (int row = 0; row < text_rows; ++row) {
        out += "\x1b[K";
        for (int col = 0; col < size.cols; ++col) {
            out.push_back(editor.world.block_at({col, editor.cursor.y, row}));
        }
        out += "\r\n";
    }

    out += "\x1b[7m";
    std::string status = std::string(" ") + mode_name(editor.mode)
        + "  x=" + std::to_string(editor.cursor.x)
        + " y=" + std::to_string(editor.cursor.y)
        + " z=" + std::to_string(editor.cursor.z)
        + " dir=" + direction_name(editor.direction);
    if (editor.dirty) {
        status += " [+]";
    }
    if (!editor.message.empty() && editor.mode != Mode::Command) {
        status += "  " + editor.message;
    }
    out += crop_line(status, size.cols);
    if (static_cast<int>(status.size()) < size.cols) {
        out.append(static_cast<std::size_t>(size.cols - status.size()), ' ');
    }
    out += "\x1b[0m";

    if (editor.mode == Mode::Command) {
        out += "\x1b[";
        out += std::to_string(size.rows);
        out += ";1H\x1b[K:";
        out += crop_line(editor.command, size.cols - 1);
    }

    int screen_row = clamp_int(editor.cursor.z + 1, 1, text_rows);
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

void load_initial_file(Editor& editor, const char* filename) {
    if (filename == nullptr || std::string_view(filename).empty()) {
        return;
    }
    try {
        mcvi::read_world({editor.world, filename, mcvi::format_from_extension(filename)});
        editor.filename = filename;
        editor.message = "read " + editor.filename;
    } catch (const std::exception& error) {
        editor.filename = filename;
        editor.message = error.what();
    }
}

} // namespace

int main(int argc, char** argv) {
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    RawTerminal terminal;
    Editor editor;
    if (argc > 1) {
        load_initial_file(editor, argv[1]);
    }

    while (editor.running && !g_interrupted) {
        render(editor);
        char ch = read_key();
        if (ch != 0) {
            handle_key(editor, ch);
        }
    }

    return 0;
}
