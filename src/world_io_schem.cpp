#include "world_io_formats.hpp"

#include "world_io_common.hpp"

namespace mcvi {

void read_schem_world(const WorldReadRequest&) {
    io::unsupported_format(WorldFormat::Schem, "read");
}

void write_schem_world(const WorldWriteRequest&) {
    io::unsupported_format(WorldFormat::Schem, "write");
}

} // namespace mcvi
