#include "input.hpp"

#include <cctype>

#include "command.hpp"
#include "direction.hpp"
#include "mode.hpp"

namespace mcvi {
namespace {

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

} // namespace

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

} // namespace mcvi
