#include "world_io_formats.hpp"

#include "world_io_common.hpp"

namespace mcvi {

void read_nbt_world(const WorldReadRequest&) {
    io::unsupported_format(WorldFormat::Nbt, "read");
}

void write_nbt_world(const WorldWriteRequest&) {
    io::unsupported_format(WorldFormat::Nbt, "write");
}

} // namespace mcvi
