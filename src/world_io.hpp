#pragma once

#include <string>
#include <string_view>

namespace mcvi {

class World;

enum class WorldFormat {
    Schem,
    Nbt,
    Mca,
    Json,
};

struct WorldReadRequest {
    World& data;
    std::string filename;
    WorldFormat format;
};

struct WorldWriteRequest {
    const World& data;
    std::string filename;
    WorldFormat format;
};

WorldFormat format_from_extension(std::string_view filename);

void read_world(const WorldReadRequest& request);
void write_world(const WorldWriteRequest& request);

} // namespace mcvi
