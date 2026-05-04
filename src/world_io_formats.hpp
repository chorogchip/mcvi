#pragma once

#include "world_io.hpp"

namespace mcvi {

void read_json_world(const WorldReadRequest& request);
void write_json_world(const WorldWriteRequest& request);

void read_schem_world(const WorldReadRequest& request);
void write_schem_world(const WorldWriteRequest& request);

void read_nbt_world(const WorldReadRequest& request);
void write_nbt_world(const WorldWriteRequest& request);

void read_mca_world(const WorldReadRequest& request);
void write_mca_world(const WorldWriteRequest& request);

} // namespace mcvi
