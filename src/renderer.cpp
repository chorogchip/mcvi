#include "renderer.hpp"

#include <algorithm>
#include <string>
#include <string_view>

#include "direction.hpp"
#include "mode.hpp"
#include "terminal.hpp"

namespace mcvi {
namespace {

struct ColumnPreviewCell {
    char block = ' ';
    bool is_cursor_y = false;
};

int clamp_int(int value, int low, int high) {
    return std::max(low, std::min(value, high));
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

ColumnPreviewCell column_preview_cell(const Editor& editor, int row, int text_rows) {
    if (editor.world.empty()) {
        return {};
    }

    int min_y = editor.world.min_y();
    int max_y = editor.world.max_y();
    int preview_rows = max_y - min_y + 1;
    int start_row = (text_rows - preview_rows) / 2;
    int preview_index = row - start_row;
    if (preview_index < 0 || preview_index >= preview_rows) {
        return {};
    }

    int y = max_y - preview_index;
    return {editor.world.block_at({editor.cursor.x, y, editor.cursor.z}), y == editor.cursor.y};
}

} // namespace

void render(const Editor& editor) {
    TerminalSize size = terminal_size();
    int text_rows = std::max(1, size.rows - 1);
    int edit_cols = std::max(0, size.cols - 1);
    std::string out;
    out.reserve(static_cast<std::size_t>(size.rows * size.cols + 128));

    out += "\x1b[?25l\x1b[H";
    for (int row = 0; row < text_rows; ++row) {
        out += "\x1b[K";
        for (int col = 0; col < edit_cols; ++col) {
            out.push_back(editor.world.block_at({col, editor.cursor.y, row}));
        }
        ColumnPreviewCell preview = column_preview_cell(editor, row, text_rows);
        out += preview.is_cursor_y ? "\x1b[44;37m" : "\x1b[7m";
        out.push_back(preview.block);
        out += "\x1b[0m";
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
    int screen_col = clamp_int(editor.cursor.x + 1, 1, std::max(1, edit_cols));
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

} // namespace mcvi
