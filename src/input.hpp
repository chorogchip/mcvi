#pragma once

#include "editor.hpp"

namespace mcvi {

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

void handle_key(Editor& editor, char ch);

} // namespace mcvi
