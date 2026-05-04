#pragma once

#include <cstddef>

namespace mcvi {

struct Pos {
    int x = 0;
    int y = 0;
    int z = 0;

    friend bool operator==(const Pos& lhs, const Pos& rhs) {
        return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
    }
};

inline Pos operator+(Pos lhs, Pos rhs) {
    return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

inline Pos operator-(Pos lhs, Pos rhs) {
    return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

struct PosHash {
    std::size_t operator()(const Pos& pos) const {
        std::size_t h = static_cast<std::size_t>(pos.x);
        h = h * 1315423911u + static_cast<std::size_t>(pos.y);
        h = h * 1315423911u + static_cast<std::size_t>(pos.z);
        return h;
    }
};

} // namespace mcvi
