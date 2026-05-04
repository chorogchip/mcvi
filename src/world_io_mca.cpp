#include "world_io_formats.hpp"

#include "world_io_common.hpp"

namespace mcvi {

void read_mca_world(const WorldReadRequest&) {
    io::unsupported_format(WorldFormat::Mca, "read");
}

void write_mca_world(const WorldWriteRequest&) {
    io::unsupported_format(WorldFormat::Mca, "write");
}

} // namespace mcvi
