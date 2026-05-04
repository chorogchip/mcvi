#pragma once

#include "pos.hpp"

namespace mcvi {

enum class Direction {
    PosX,
    NegX,
    PosY,
    NegY,
    PosZ,
    NegZ,
};

Pos direction_delta(Direction direction);
const char* direction_name(Direction direction);

} // namespace mcvi
