#include "app_args.hpp"

#include <iostream>
#include <stdexcept>
#include <string_view>

#include "editor.hpp"
#include "input.hpp"

namespace mcvi {
namespace {

std::string decode_auto_input(std::string_view input) {
    std::string out;
    for (std::size_t i = 0; i < input.size(); ++i) {
        char ch = input[i];
        if (ch != '\\' || i + 1 == input.size()) {
            out.push_back(ch);
            continue;
        }

        char escaped = input[++i];
        switch (escaped) {
        case 'n':
        case 'r':
            out.push_back(ENTER);
            break;
        case 'e':
            out.push_back(ESC);
            break;
        case 'b':
            out.push_back(BACKSPACE);
            break;
        case 't':
            out.push_back('\t');
            break;
        case '\\':
            out.push_back('\\');
            break;
        default:
            out.push_back(escaped);
            break;
        }
    }
    return out;
}

} // namespace

void print_usage(const char* program) {
    std::cout << "Usage: " << program << " [-auto INPUT] [file]\n"
              << "Auto input escapes: \\\\n newline, \\\\r enter, \\\\e escape, \\\\b backspace, \\\\t tab.\n";
}

AppArgs parse_args(int argc, char** argv) {
    AppArgs args;
    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            args.help = true;
            continue;
        }
        if (arg == "-auto") {
            if (i + 1 >= argc) {
                throw std::runtime_error("-auto requires an input string");
            }
            args.auto_mode = true;
            args.auto_input = argv[++i];
            continue;
        }
        if (args.filename.empty()) {
            args.filename = arg;
            continue;
        }
        throw std::runtime_error("unexpected argument: " + std::string(arg));
    }
    return args;
}

int run_auto(const AppArgs& args) {
    Editor editor;
    if (!args.filename.empty()) {
        load_initial_file(editor, args.filename.c_str());
    }

    for (char ch : decode_auto_input(args.auto_input)) {
        if (!editor.running) {
            break;
        }
        handle_key(editor, ch);
    }

    if (!editor.message.empty()) {
        std::cout << editor.message << '\n';
    }
    return 0;
}

} // namespace mcvi
