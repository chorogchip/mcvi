#include "editor.hpp"

#include <algorithm>
#include <exception>
#include <string_view>

#include "world_io.hpp"

namespace mcvi {

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

void load_initial_file(Editor& editor, const char* filename) {
    if (filename == nullptr || std::string_view(filename).empty()) {
        return;
    }
    try {
        read_world({editor.world, filename, format_from_extension(filename)});
        editor.filename = filename;
        editor.message = "read " + editor.filename;
    } catch (const std::exception& error) {
        editor.filename = filename;
        editor.message = error.what();
    }
}

} // namespace mcvi
