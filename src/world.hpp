#pragma once

#include <algorithm>
#include <unordered_map>

#include "pos.hpp"

namespace mcvi {

class World {
public:
    using Blocks = std::unordered_map<Pos, char, PosHash>;

    char block_at(Pos pos) const {
        auto it = blocks_.find(pos);
        if (it == blocks_.end()) {
            return ' ';
        }
        return it->second;
    }

    void set_block(Pos pos, char block) {
        if (block == ' ') {
            blocks_.erase(pos);
            return;
        }
        blocks_[pos] = block;
    }

    void clear() {
        blocks_.clear();
    }

    const Blocks& blocks() const {
        return blocks_;
    }

    bool empty() const {
        return blocks_.empty();
    }

    int max_x_on_row(int y, int z) const {
        int max_x = 0;
        for (const auto& [pos, block] : blocks_) {
            if (block != ' ' && pos.y == y && pos.z == z) {
                max_x = std::max(max_x, pos.x);
            }
        }
        return max_x;
    }

    int min_y() const {
        if (blocks_.empty()) {
            return 0;
        }

        int min_value = blocks_.begin()->first.y;
        for (const auto& [pos, block] : blocks_) {
            if (block != ' ') {
                min_value = std::min(min_value, pos.y);
            }
        }
        return min_value;
    }

    int max_y() const {
        if (blocks_.empty()) {
            return 0;
        }

        int max_value = blocks_.begin()->first.y;
        for (const auto& [pos, block] : blocks_) {
            if (block != ' ') {
                max_value = std::max(max_value, pos.y);
            }
        }
        return max_value;
    }

private:
    Blocks blocks_;
};

} // namespace mcvi
