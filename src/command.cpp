#include "command.hpp"

#include <cctype>
#include <exception>
#include <stdexcept>
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
        write_world({editor.world, &editor.aliases, filename, format_from_extension(filename), &editor.metadata});
    } catch (const std::exception& error) {
        editor.message = error.what();
        return false;
    }

    editor.filename = std::move(filename);
    editor.dirty = false;
    editor.message = "written " + editor.filename;
    return true;
}

std::string metadata_description(const SchematicMetadata& metadata) {
    return "meta name=\"" + metadata.name + "\" author=\"" + metadata.author
        + "\" version=" + std::to_string(metadata.data_version);
}

bool handle_metadata_command(Editor& editor, std::string_view argument) {
    std::string text = trim(argument);
    if (text.empty() || text == "all") {
        editor.message = metadata_description(editor.metadata);
        return true;
    }

    std::size_t space = text.find_first_of(" \t");
    std::string key = space == std::string::npos ? text : text.substr(0, space);
    std::string value = space == std::string::npos ? "" : trim(std::string_view(text).substr(space + 1));
    if (value.empty()) {
        editor.message = "Usage: meta name|author|version value";
        return false;
    }

    if (key == "name") {
        editor.metadata.name = value;
    } else if (key == "author") {
        editor.metadata.author = value;
    } else if (key == "version" || key == "data_version" || key == "mc_version") {
        try {
            editor.metadata.data_version = std::stoi(value);
        } catch (const std::exception&) {
            editor.message = "metadata version must be an integer";
            return false;
        }
    } else {
        editor.message = "Unknown metadata key: " + key;
        return false;
    }

    editor.message = metadata_description(editor.metadata);
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
    if (command == "meta" || command.starts_with("meta ")) {
        (void)handle_metadata_command(editor, command_argument(command, "meta"));
        return;
    }

    editor.message = "Not an editor command: " + command;
}

} // namespace mcvi
