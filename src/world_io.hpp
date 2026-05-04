#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>

namespace mcvi {

struct Pos {
    int x = 0;
    int y = 0;
    int z = 0;

    friend bool operator==(const Pos&, const Pos&) = default;
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

enum class WorldFormat {
    Schem,
    Nbt,
    Mca,
    Json,
};

struct WorldReadRequest {
    World& data;
    std::string filename;
    WorldFormat format;
};

struct WorldWriteRequest {
    const World& data;
    std::string filename;
    WorldFormat format;
};

WorldFormat format_from_extension(std::string_view filename);

void read_world(const WorldReadRequest& request);
void write_world(const WorldWriteRequest& request);

} // namespace mcvi
