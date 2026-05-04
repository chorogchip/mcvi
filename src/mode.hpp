#pragma once

namespace mcvi {

enum class Mode {
    Normal,
    Insert,
    Command,
};

const char* mode_name(Mode mode);

} // namespace mcvi
