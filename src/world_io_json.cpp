#include "world_io_formats.hpp"

#include "world_io_common.hpp"

#include <cctype>
#include <fstream>
#include <stdexcept>
#include <string>

namespace mcvi {
namespace {

std::string escape_json_char(char ch) {
    switch (ch) {
    case '\\':
        return "\\\\";
    case '"':
        return "\\\"";
    case '\n':
        return "\\n";
    case '\r':
        return "\\r";
    case '\t':
        return "\\t";
    default:
        if (static_cast<unsigned char>(ch) < 0x20) {
            throw std::runtime_error("cannot write control character block");
        }
        return std::string(1, ch);
    }
}

class JsonReader {
public:
    explicit JsonReader(std::string data)
        : data_(std::move(data)) {}

    World read_world() {
        World world;
        expect('{');
        while (!consume('}')) {
            std::string key = read_string();
            expect(':');
            if (key == "blocks") {
                read_blocks(world);
            } else {
                skip_value();
            }
            consume(',');
        }
        skip_ws();
        if (pos_ != data_.size()) {
            fail("unexpected trailing data");
        }
        return world;
    }

private:
    void read_blocks(World& world) {
        expect('[');
        while (!consume(']')) {
            read_block(world);
            consume(',');
        }
    }

    void read_block(World& world) {
        Pos pos;
        bool has_x = false;
        bool has_y = false;
        bool has_z = false;
        bool has_block = false;
        char block = ' ';

        expect('{');
        while (!consume('}')) {
            std::string key = read_string();
            expect(':');
            if (key == "x") {
                pos.x = read_int();
                has_x = true;
            } else if (key == "y") {
                pos.y = read_int();
                has_y = true;
            } else if (key == "z") {
                pos.z = read_int();
                has_z = true;
            } else if (key == "block") {
                std::string value = read_string();
                if (value.size() != 1) {
                    fail("block must be a one-character string");
                }
                block = value[0];
                has_block = true;
            } else {
                skip_value();
            }
            consume(',');
        }

        if (!has_x || !has_y || !has_z || !has_block) {
            fail("block entry is missing x, y, z, or block");
        }
        world.set_block(pos, block);
    }

    void skip_value() {
        skip_ws();
        char ch = peek();
        if (ch == '"') {
            (void)read_string();
            return;
        }
        if (ch == '-' || std::isdigit(static_cast<unsigned char>(ch))) {
            (void)read_int();
            return;
        }
        if (ch == '{') {
            expect('{');
            while (!consume('}')) {
                (void)read_string();
                expect(':');
                skip_value();
                consume(',');
            }
            return;
        }
        if (ch == '[') {
            expect('[');
            while (!consume(']')) {
                skip_value();
                consume(',');
            }
            return;
        }
        if (starts_with("true")) {
            pos_ += 4;
            return;
        }
        if (starts_with("false")) {
            pos_ += 5;
            return;
        }
        if (starts_with("null")) {
            pos_ += 4;
            return;
        }
        fail("invalid json value");
    }

    std::string read_string() {
        expect('"');
        std::string out;
        while (pos_ < data_.size()) {
            char ch = data_[pos_++];
            if (ch == '"') {
                return out;
            }
            if (ch != '\\') {
                out.push_back(ch);
                continue;
            }
            if (pos_ == data_.size()) {
                fail("unterminated escape sequence");
            }
            char escaped = data_[pos_++];
            switch (escaped) {
            case '"':
            case '\\':
            case '/':
                out.push_back(escaped);
                break;
            case 'n':
                out.push_back('\n');
                break;
            case 'r':
                out.push_back('\r');
                break;
            case 't':
                out.push_back('\t');
                break;
            default:
                fail("unsupported string escape");
            }
        }
        fail("unterminated string");
    }

    int read_int() {
        skip_ws();
        std::size_t start = pos_;
        if (peek() == '-') {
            ++pos_;
        }
        if (pos_ == data_.size() || !std::isdigit(static_cast<unsigned char>(data_[pos_]))) {
            fail("expected integer");
        }
        while (pos_ < data_.size() && std::isdigit(static_cast<unsigned char>(data_[pos_]))) {
            ++pos_;
        }
        return std::stoi(data_.substr(start, pos_ - start));
    }

    bool starts_with(std::string_view needle) const {
        return data_.compare(pos_, needle.size(), needle) == 0;
    }

    char peek() {
        skip_ws();
        if (pos_ == data_.size()) {
            fail("unexpected end of file");
        }
        return data_[pos_];
    }

    bool consume(char expected) {
        skip_ws();
        if (pos_ < data_.size() && data_[pos_] == expected) {
            ++pos_;
            return true;
        }
        return false;
    }

    void expect(char expected) {
        if (!consume(expected)) {
            fail(std::string("expected '") + expected + "'");
        }
    }

    void skip_ws() {
        while (pos_ < data_.size() && std::isspace(static_cast<unsigned char>(data_[pos_]))) {
            ++pos_;
        }
    }

    [[noreturn]] void fail(std::string_view message) const {
        throw std::runtime_error("json parse error at byte " + std::to_string(pos_) + ": " + std::string(message));
    }

    std::string data_;
    std::size_t pos_ = 0;
};

} // namespace

void read_json_world(const WorldReadRequest& request) {
    request.data = JsonReader(io::read_text_file(request.filename)).read_world();
}

void write_json_world(const WorldWriteRequest& request) {
    std::ofstream output(request.filename);
    if (!output) {
        throw std::runtime_error("could not open " + request.filename + " for writing");
    }

    std::vector<std::pair<Pos, char>> blocks = io::sorted_blocks(request.data);
    output << "{\n";
    output << "  \"format\": \"mcvi-json-v1\",\n";
    output << "  \"blocks\": [\n";
    for (std::size_t i = 0; i < blocks.size(); ++i) {
        const auto& [pos, block] = blocks[i];
        output << "    {\"x\": " << pos.x
               << ", \"y\": " << pos.y
               << ", \"z\": " << pos.z
               << ", \"block\": \"" << escape_json_char(block) << "\"}";
        if (i + 1 != blocks.size()) {
            output << ',';
        }
        output << '\n';
    }
    output << "  ]\n";
    output << "}\n";

    if (!output) {
        throw std::runtime_error("failed while writing " + request.filename);
    }
}

} // namespace mcvi
