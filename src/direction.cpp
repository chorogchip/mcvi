#include "direction.hpp"

namespace mcvi {

Pos direction_delta(Direction direction) {
    switch (direction) {
    case Direction::PosX:
        return {1, 0, 0};
    case Direction::NegX:
        return {-1, 0, 0};
    case Direction::PosY:
        return {0, 1, 0};
    case Direction::NegY:
        return {0, -1, 0};
    case Direction::PosZ:
        return {0, 0, 1};
    case Direction::NegZ:
        return {0, 0, -1};
    }
    return {1, 0, 0};
}

const char* direction_name(Direction direction) {
    switch (direction) {
    case Direction::PosX:
        return "+x";
    case Direction::NegX:
        return "-x";
    case Direction::PosY:
        return "+y";
    case Direction::NegY:
        return "-y";
    case Direction::PosZ:
        return "+z";
    case Direction::NegZ:
        return "-z";
    }
    return "";
}

} // namespace mcvi
