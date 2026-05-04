#pragma once

#include <string>
#include <string_view>

#include "block_alias.hpp"
#include "world.hpp"

namespace mcvi {

enum class WorldFormat {
    Schem,
    Nbt,
    Mca,
    Json,
};

struct WorldReadRequest {
    World& data;
    BlockAliases* aliases = nullptr;
    std::string filename;
    WorldFormat format;
};

struct WorldWriteRequest {
    const World& data;
    const BlockAliases* aliases = nullptr;
    std::string filename;
    WorldFormat format;
};

WorldFormat format_from_extension(std::string_view filename);

void read_world(const WorldReadRequest& request);
void write_world(const WorldWriteRequest& request);

} // namespace mcvi
