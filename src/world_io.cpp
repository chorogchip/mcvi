#include "world_io.hpp"

#include "world_io_common.hpp"
#include "world_io_formats.hpp"

namespace mcvi {

WorldFormat format_from_extension(std::string_view filename) {
    std::string extension = io::lower_extension(filename);
    if (extension == "schem" || extension == "schematic") {
        return WorldFormat::Schem;
    }
    if (extension == "nbt") {
        return WorldFormat::Nbt;
    }
    if (extension == "mca") {
        return WorldFormat::Mca;
    }
    return WorldFormat::Json;
}

void read_world(const WorldReadRequest& request) {
    switch (request.format) {
    case WorldFormat::Schem:
        read_schem_world(request);
        return;
    case WorldFormat::Nbt:
        read_nbt_world(request);
        return;
    case WorldFormat::Mca:
        read_mca_world(request);
        return;
    case WorldFormat::Json:
        read_json_world(request);
        return;
    }
}

void write_world(const WorldWriteRequest& request) {
    switch (request.format) {
    case WorldFormat::Schem:
        write_schem_world(request);
        return;
    case WorldFormat::Nbt:
        write_nbt_world(request);
        return;
    case WorldFormat::Mca:
        write_mca_world(request);
        return;
    case WorldFormat::Json:
        write_json_world(request);
        return;
    }
}

} // namespace mcvi
