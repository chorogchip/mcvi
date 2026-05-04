#include "world_io_formats.hpp"

#include "nbt_writer.hpp"
#include "world_io_common.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace mcvi {
namespace {

constexpr std::int32_t kSpongeSchematicVersion = 3;

struct Bounds {
    int min_x = 0;
    int min_y = 0;
    int min_z = 0;
    int max_x = 0;
    int max_y = 0;
    int max_z = 0;
};

Bounds compute_bounds(const World& world) {
    if (world.empty()) {
        throw std::runtime_error("cannot export empty world");
    }

    auto it = world.blocks().begin();
    Bounds bounds{it->first.x, it->first.y, it->first.z, it->first.x, it->first.y, it->first.z};
    for (const auto& [pos, block] : world.blocks()) {
        (void)block;
        bounds.min_x = std::min(bounds.min_x, pos.x);
        bounds.min_y = std::min(bounds.min_y, pos.y);
        bounds.min_z = std::min(bounds.min_z, pos.z);
        bounds.max_x = std::max(bounds.max_x, pos.x);
        bounds.max_y = std::max(bounds.max_y, pos.y);
        bounds.max_z = std::max(bounds.max_z, pos.z);
    }
    return bounds;
}

std::int16_t dimension_to_short(int value, std::string_view name) {
    if (value <= 0 || value > std::numeric_limits<std::uint16_t>::max()) {
        throw std::runtime_error(std::string(name) + " is out of range for schematic export");
    }
    return static_cast<std::int16_t>(static_cast<std::uint16_t>(value));
}

std::string block_state_for(char block, const BlockAliases* aliases) {
    if (aliases == nullptr) {
        throw std::runtime_error(std::string("missing block alias for ") + block);
    }
    const std::string* state = aliases->get(block);
    if (state == nullptr || state->empty()) {
        throw std::runtime_error(std::string("missing block alias for ") + block);
    }
    return *state;
}

void write_varint(std::vector<std::uint8_t>& out, std::uint32_t value) {
    do {
        std::uint8_t temp = value & 0x7F;
        value >>= 7;
        if (value != 0) {
            temp |= 0x80;
        }
        out.push_back(temp);
    } while (value != 0);
}

void write_file_bytes(const std::string& filename, const std::vector<std::uint8_t>& bytes) {
    std::ofstream output(filename, std::ios::binary);
    if (!output) {
        throw std::runtime_error("could not open " + filename + " for writing");
    }
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!output) {
        throw std::runtime_error("failed while writing " + filename);
    }
}

} // namespace

void read_schem_world(const WorldReadRequest&) {
    io::unsupported_format(WorldFormat::Schem, "read");
}

void write_schem_world(const WorldWriteRequest& request) {
    SchematicMetadata default_metadata;
    const SchematicMetadata& metadata = request.metadata == nullptr ? default_metadata : *request.metadata;

    Bounds bounds = compute_bounds(request.data);
    int width = bounds.max_x - bounds.min_x + 1;
    int height = bounds.max_y - bounds.min_y + 1;
    int length = bounds.max_z - bounds.min_z + 1;
    std::int16_t nbt_width = dimension_to_short(width, "width");
    std::int16_t nbt_height = dimension_to_short(height, "height");
    std::int16_t nbt_length = dimension_to_short(length, "length");

    std::set<std::string> used_states;
    std::vector<std::pair<Pos, std::string>> block_states;
    block_states.reserve(request.data.blocks().size());
    for (const auto& [pos, block] : request.data.blocks()) {
        std::string state = block_state_for(block, request.aliases);
        used_states.insert(state);
        block_states.push_back({pos, std::move(state)});
    }

    std::map<std::string, std::int32_t> palette;
    palette.emplace("minecraft:air", 0);
    std::int32_t next_palette_index = 1;
    for (const std::string& state : used_states) {
        if (state == "minecraft:air") {
            continue;
        }
        palette.emplace(state, next_palette_index++);
    }

    std::vector<std::int32_t> block_indices(static_cast<std::size_t>(width) * height * length, 0);
    for (const auto& [pos, state] : block_states) {
        int local_x = pos.x - bounds.min_x;
        int local_y = pos.y - bounds.min_y;
        int local_z = pos.z - bounds.min_z;
        std::size_t index = static_cast<std::size_t>(local_x)
            + static_cast<std::size_t>(local_z) * width
            + static_cast<std::size_t>(local_y) * width * length;
        block_indices[index] = palette.at(state);
    }

    std::vector<std::uint8_t> block_data;
    for (std::int32_t index : block_indices) {
        write_varint(block_data, static_cast<std::uint32_t>(index));
    }

    nbt::Writer writer;
    writer.begin_root_compound();
    writer.begin_compound("Schematic");
    writer.write_int("Version", kSpongeSchematicVersion);
    writer.write_int("DataVersion", metadata.data_version);
    writer.write_short("Width", nbt_width);
    writer.write_short("Height", nbt_height);
    writer.write_short("Length", nbt_length);
    writer.write_int_array("Offset", {0, 0, 0});
    writer.begin_compound("Blocks");
    writer.begin_compound("Palette");
    for (const auto& [state, index] : palette) {
        writer.write_int(state, index);
    }
    writer.end_compound();
    writer.write_byte_array("Data", block_data);
    writer.begin_list("BlockEntities", nbt::Tag::End, 0);
    writer.end_compound();
    writer.begin_list("Entities", nbt::Tag::End, 0);
    writer.begin_compound("Metadata");
    writer.write_string("Name", metadata.name);
    writer.write_string("Author", metadata.author);
    writer.write_int("mc_version", metadata.data_version);
    writer.end_compound();
    writer.end_compound();
    writer.end_compound();

    write_file_bytes(request.filename, nbt::gzip_compress(writer.bytes()));
}

} // namespace mcvi
