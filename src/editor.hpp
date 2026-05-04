#pragma once

#include <string>

#include "block_alias.hpp"
#include "direction.hpp"
#include "mode.hpp"
#include "pos.hpp"
#include "world.hpp"
#include "world_io.hpp"

namespace mcvi {

struct Editor {
    Mode mode = Mode::Normal;
    World world;
    BlockAliases aliases;
    SchematicMetadata metadata;
    Pos cursor;
    Direction direction = Direction::PosX;
    std::string command;
    std::string filename;
    std::string message = "mcvi slice y=0";
    bool dirty = false;
    bool running = true;
};

void move_left(Editor& editor);
void move_right(Editor& editor);
void move_up(Editor& editor);
void move_down(Editor& editor);
void move_layer_up(Editor& editor);
void move_layer_down(Editor& editor);
void move_first_non_blank(Editor& editor);
void move_after_end_of_row(Editor& editor);

void insert_char(Editor& editor, char ch);
void insert_newline(Editor& editor);
void backspace(Editor& editor);
void delete_at_cursor(Editor& editor);
void delete_before_cursor(Editor& editor);
void delete_to_end_of_row(Editor& editor);
void change_to_end_of_row(Editor& editor);
void substitute_at_cursor(Editor& editor);
void substitute_row(Editor& editor);
void set_direction(Editor& editor, Direction direction);
void load_initial_file(Editor& editor, const char* filename);

} // namespace mcvi
