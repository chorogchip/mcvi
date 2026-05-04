#include "editor.hpp"

#include <algorithm>
#include <exception>
#include <string_view>

#include "world_io.hpp"

namespace mcvi {
namespace {

bool row_bounds(const World& world, int y, int z, int& min_x, int& max_x) {
    bool found = false;
    for (const auto& [pos, block] : world.blocks()) {
        if (block == ' ' || pos.y != y || pos.z != z) {
            continue;
        }
        if (!found) {
            min_x = pos.x;
            max_x = pos.x;
            found = true;
            continue;
        }
        min_x = std::min(min_x, pos.x);
        max_x = std::max(max_x, pos.x);
    }
    return found;
}

} // namespace

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

void move_first_non_blank(Editor& editor) {
    int min_x = 0;
    int max_x = 0;
    if (row_bounds(editor.world, editor.cursor.y, editor.cursor.z, min_x, max_x)) {
        editor.cursor.x = std::max(0, min_x);
        return;
    }
    editor.cursor.x = 0;
}

void move_after_end_of_row(Editor& editor) {
    int min_x = 0;
    int max_x = 0;
    if (row_bounds(editor.world, editor.cursor.y, editor.cursor.z, min_x, max_x)) {
        editor.cursor.x = std::max(0, max_x + 1);
        return;
    }
    editor.cursor.x = 0;
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

void delete_at_cursor(Editor& editor) {
    if (editor.world.block_at(editor.cursor) == ' ') {
        return;
    }
    editor.world.set_block(editor.cursor, ' ');
    editor.dirty = true;
}

void delete_before_cursor(Editor& editor) {
    if (editor.cursor.x <= 0) {
        return;
    }
    --editor.cursor.x;
    delete_at_cursor(editor);
}

void delete_to_end_of_row(Editor& editor) {
    int end_x = editor.world.max_x_on_row(editor.cursor.y, editor.cursor.z);
    if (editor.cursor.x > end_x) {
        return;
    }

    bool changed = false;
    for (int x = editor.cursor.x; x <= end_x; ++x) {
        Pos pos{x, editor.cursor.y, editor.cursor.z};
        if (editor.world.block_at(pos) != ' ') {
            editor.world.set_block(pos, ' ');
            changed = true;
        }
    }
    if (changed) {
        editor.dirty = true;
    }
}

void change_to_end_of_row(Editor& editor) {
    delete_to_end_of_row(editor);
    editor.mode = Mode::Insert;
}

void substitute_at_cursor(Editor& editor) {
    delete_at_cursor(editor);
    editor.mode = Mode::Insert;
}

void substitute_row(Editor& editor) {
    move_first_non_blank(editor);
    delete_to_end_of_row(editor);
    editor.mode = Mode::Insert;
}

void set_direction(Editor& editor, Direction direction) {
    editor.direction = direction;
    editor.message = std::string("direction ") + direction_name(direction);
}

void load_initial_file(Editor& editor, const char* filename) {
    if (filename == nullptr || std::string_view(filename).empty()) {
        return;
    }
    try {
        read_world({editor.world, &editor.aliases, filename, format_from_extension(filename)});
        editor.filename = filename;
        editor.message = "read " + editor.filename;
    } catch (const std::exception& error) {
        editor.filename = filename;
        editor.message = error.what();
    }
}

} // namespace mcvi
