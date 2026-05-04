#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace mcvi::nbt {

enum class Tag : std::uint8_t {
    End = 0,
    Byte = 1,
    Short = 2,
    Int = 3,
    Long = 4,
    ByteArray = 7,
    String = 8,
    List = 9,
    Compound = 10,
    IntArray = 11,
};

class Writer {
public:
    void begin_root_compound(std::string_view name = "");
    void end_compound();

    void write_byte(std::string_view name, std::int8_t value);
    void write_short(std::string_view name, std::int16_t value);
    void write_int(std::string_view name, std::int32_t value);
    void write_long(std::string_view name, std::int64_t value);
    void write_string(std::string_view name, std::string_view value);
    void write_byte_array(std::string_view name, const std::vector<std::uint8_t>& value);
    void write_int_array(std::string_view name, const std::vector<std::int32_t>& value);

    void begin_list(std::string_view name, Tag element_type, std::int32_t length);
    void begin_compound(std::string_view name);

    const std::vector<std::uint8_t>& bytes() const {
        return bytes_;
    }

private:
    void write_named_header(Tag tag, std::string_view name);
    void write_u8(std::uint8_t value);
    void write_i16(std::int16_t value);
    void write_i32(std::int32_t value);
    void write_i64(std::int64_t value);
    void write_string_payload(std::string_view value);

    std::vector<std::uint8_t> bytes_;
};

std::vector<std::uint8_t> gzip_compress(const std::vector<std::uint8_t>& input);

} // namespace mcvi::nbt
