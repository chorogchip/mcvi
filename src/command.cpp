#include "command.hpp"

#include <cctype>
#include <exception>
#include <string>
#include <string_view>

#include "block_alias.hpp"
#include "mode.hpp"
#include "world_io.hpp"

namespace mcvi {
namespace {

std::string trim(std::string_view text) {
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) {
        text.remove_prefix(1);
    }
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) {
        text.remove_suffix(1);
    }
    return std::string(text);
}

std::string command_argument(std::string_view command, std::string_view name) {
    if (!command.starts_with(name)) {
        return "";
    }
    command.remove_prefix(name.size());
    return trim(command);
}

bool write_editor(Editor& editor, std::string filename) {
    if (filename.empty()) {
        filename = editor.filename;
    }
    if (filename.empty()) {
        editor.message = "No file name";
        return false;
    }

    try {
        write_world({editor.world, &editor.aliases, filename, format_from_extension(filename)});
    } catch (const std::exception& error) {
        editor.message = error.what();
        return false;
    }

    editor.filename = std::move(filename);
    editor.dirty = false;
    editor.message = "written " + editor.filename;
    return true;
}

} // namespace

void execute_command(Editor& editor) {
    std::string command = trim(editor.command);
    editor.command.clear();
    editor.mode = Mode::Normal;

    if (command == "q") {
        if (editor.dirty) {
            editor.message = "No write since last change";
            return;
        }
        editor.running = false;
        return;
    }
    if (command == "q!") {
        editor.running = false;
        return;
    }
    if (command == "w") {
        (void)write_editor(editor, "");
        return;
    }
    if (command.starts_with("w ")) {
        (void)write_editor(editor, command_argument(command, "w"));
        return;
    }
    if (command == "wq" || command == "x") {
        if (!editor.dirty || write_editor(editor, "")) {
            editor.running = false;
        }
        return;
    }
    if (command == "bl" || command.starts_with("bl ")) {
        (void)handle_block_alias_command(editor.aliases, command_argument(command, "bl"), editor.message);
        return;
    }

    editor.message = "Not an editor command: " + command;
}

} // namespace mcvi
