#include "mode.hpp"

namespace mcvi {

const char* mode_name(Mode mode) {
    switch (mode) {
    case Mode::Normal:
        return "NORMAL";
    case Mode::Insert:
        return "INSERT";
    case Mode::Command:
        return "COMMAND";
    }
    return "";
}

} // namespace mcvi
