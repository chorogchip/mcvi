#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "pos.hpp"
#include "world.hpp"
#include "world_io.hpp"

namespace mcvi::io {

std::string lower_extension(std::string_view filename);
std::string read_text_file(const std::string& filename);
std::vector<std::pair<Pos, char>> sorted_blocks(const World& world);

[[noreturn]] void unsupported_format(WorldFormat format, std::string_view operation);

} // namespace mcvi::io
