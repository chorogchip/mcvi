#include "world_io_common.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <stdexcept>

namespace mcvi::io {

std::string lower_extension(std::string_view filename) {
    std::size_t slash = filename.find_last_of("/\\");
    std::size_t dot = filename.find_last_of('.');
    if (dot == std::string_view::npos || (slash != std::string_view::npos && dot < slash)) {
        return "";
    }

    std::string extension(filename.substr(dot + 1));
    for (char& ch : extension) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return extension;
}

std::string read_text_file(const std::string& filename) {
    std::ifstream input(filename);
    if (!input) {
        throw std::runtime_error("could not open " + filename + " for reading");
    }
    return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

std::vector<std::pair<Pos, char>> sorted_blocks(const World& world) {
    std::vector<std::pair<Pos, char>> blocks;
    blocks.reserve(world.blocks().size());
    for (const auto& [pos, block] : world.blocks()) {
        blocks.push_back({pos, block});
    }
    std::sort(blocks.begin(), blocks.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.first.y != rhs.first.y) {
            return lhs.first.y < rhs.first.y;
        }
        if (lhs.first.z != rhs.first.z) {
            return lhs.first.z < rhs.first.z;
        }
        return lhs.first.x < rhs.first.x;
    });
    return blocks;
}

[[noreturn]] void unsupported_format(WorldFormat format, std::string_view operation) {
    const char* name = "unknown";
    switch (format) {
    case WorldFormat::Schem:
        name = "schem";
        break;
    case WorldFormat::Nbt:
        name = "nbt";
        break;
    case WorldFormat::Mca:
        name = "mca";
        break;
    case WorldFormat::Json:
        name = "json";
        break;
    }
    throw std::runtime_error(std::string(operation) + " is not implemented for " + name);
}

} // namespace mcvi::io
