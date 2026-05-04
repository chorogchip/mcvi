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

struct SchematicMetadata {
    std::string name = "mcvi export";
    std::string author = "mcvi";
    int data_version = 3953;
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
    const SchematicMetadata* metadata = nullptr;
};

WorldFormat format_from_extension(std::string_view filename);

void read_world(const WorldReadRequest& request);
void write_world(const WorldWriteRequest& request);

} // namespace mcvi
