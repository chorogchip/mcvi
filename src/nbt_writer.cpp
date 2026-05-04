#include "nbt_writer.hpp"

#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

#include <zlib.h>

namespace mcvi::nbt {

namespace {

void require_size(std::size_t size, std::size_t max, std::string_view what) {
    if (size > max) {
        throw std::runtime_error(std::string(what) + " is too large for NBT");
    }
}

} // namespace

void Writer::begin_root_compound(std::string_view name) {
    write_named_header(Tag::Compound, name);
}

void Writer::end_compound() {
    write_u8(static_cast<std::uint8_t>(Tag::End));
}

void Writer::write_byte(std::string_view name, std::int8_t value) {
    write_named_header(Tag::Byte, name);
    write_u8(static_cast<std::uint8_t>(value));
}

void Writer::write_short(std::string_view name, std::int16_t value) {
    write_named_header(Tag::Short, name);
    write_i16(value);
}

void Writer::write_int(std::string_view name, std::int32_t value) {
    write_named_header(Tag::Int, name);
    write_i32(value);
}

void Writer::write_long(std::string_view name, std::int64_t value) {
    write_named_header(Tag::Long, name);
    write_i64(value);
}

void Writer::write_string(std::string_view name, std::string_view value) {
    write_named_header(Tag::String, name);
    write_string_payload(value);
}

void Writer::write_byte_array(std::string_view name, const std::vector<std::uint8_t>& value) {
    require_size(value.size(), static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()), "byte array");
    write_named_header(Tag::ByteArray, name);
    write_i32(static_cast<std::int32_t>(value.size()));
    bytes_.insert(bytes_.end(), value.begin(), value.end());
}

void Writer::write_int_array(std::string_view name, const std::vector<std::int32_t>& value) {
    require_size(value.size(), static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()), "int array");
    write_named_header(Tag::IntArray, name);
    write_i32(static_cast<std::int32_t>(value.size()));
    for (std::int32_t item : value) {
        write_i32(item);
    }
}

void Writer::begin_list(std::string_view name, Tag element_type, std::int32_t length) {
    write_named_header(Tag::List, name);
    write_u8(static_cast<std::uint8_t>(element_type));
    write_i32(length);
}

void Writer::begin_compound(std::string_view name) {
    write_named_header(Tag::Compound, name);
}

void Writer::write_named_header(Tag tag, std::string_view name) {
    write_u8(static_cast<std::uint8_t>(tag));
    write_string_payload(name);
}

void Writer::write_u8(std::uint8_t value) {
    bytes_.push_back(value);
}

void Writer::write_i16(std::int16_t value) {
    auto unsigned_value = static_cast<std::uint16_t>(value);
    bytes_.push_back(static_cast<std::uint8_t>((unsigned_value >> 8) & 0xFF));
    bytes_.push_back(static_cast<std::uint8_t>(unsigned_value & 0xFF));
}

void Writer::write_i32(std::int32_t value) {
    auto unsigned_value = static_cast<std::uint32_t>(value);
    bytes_.push_back(static_cast<std::uint8_t>((unsigned_value >> 24) & 0xFF));
    bytes_.push_back(static_cast<std::uint8_t>((unsigned_value >> 16) & 0xFF));
    bytes_.push_back(static_cast<std::uint8_t>((unsigned_value >> 8) & 0xFF));
    bytes_.push_back(static_cast<std::uint8_t>(unsigned_value & 0xFF));
}

void Writer::write_i64(std::int64_t value) {
    auto unsigned_value = static_cast<std::uint64_t>(value);
    for (int shift = 56; shift >= 0; shift -= 8) {
        bytes_.push_back(static_cast<std::uint8_t>((unsigned_value >> shift) & 0xFF));
    }
}

void Writer::write_string_payload(std::string_view value) {
    require_size(value.size(), std::numeric_limits<std::uint16_t>::max(), "string");
    write_i16(static_cast<std::int16_t>(value.size()));
    bytes_.insert(bytes_.end(), value.begin(), value.end());
}

std::vector<std::uint8_t> gzip_compress(const std::vector<std::uint8_t>& input) {
    z_stream stream{};
    int init_result = deflateInit2(
        &stream,
        Z_DEFAULT_COMPRESSION,
        Z_DEFLATED,
        MAX_WBITS + 16,
        8,
        Z_DEFAULT_STRATEGY);
    if (init_result != Z_OK) {
        throw std::runtime_error("could not initialize gzip compressor");
    }

    stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(input.data()));
    stream.avail_in = static_cast<uInt>(input.size());

    std::vector<std::uint8_t> output;
    std::uint8_t buffer[16384];
    int result = Z_OK;
    do {
        stream.next_out = buffer;
        stream.avail_out = sizeof(buffer);
        result = deflate(&stream, Z_FINISH);
        if (result != Z_OK && result != Z_STREAM_END) {
            deflateEnd(&stream);
            throw std::runtime_error("gzip compression failed");
        }
        output.insert(output.end(), buffer, buffer + (sizeof(buffer) - stream.avail_out));
    } while (result != Z_STREAM_END);

    deflateEnd(&stream);
    return output;
}

} // namespace mcvi::nbt
