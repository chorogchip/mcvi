#include <csignal>
#include <exception>
#include <iostream>

#include "app_args.hpp"
#include "editor.hpp"
#include "input.hpp"
#include "renderer.hpp"
#include "terminal.hpp"

namespace {

volatile std::sig_atomic_t g_interrupted = 0;

void on_signal(int) {
    g_interrupted = 1;
}

} // namespace

int main(int argc, char** argv) {
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    mcvi::AppArgs args;
    try {
        args = mcvi::parse_args(argc, argv);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        mcvi::print_usage(argv[0]);
        return 2;
    }
    if (args.help) {
        mcvi::print_usage(argv[0]);
        return 0;
    }
    if (args.auto_mode) {
        return mcvi::run_auto(args);
    }

    mcvi::RawTerminal terminal;
    mcvi::Editor editor;
    if (!args.filename.empty()) {
        mcvi::load_initial_file(editor, args.filename.c_str());
    }

    while (editor.running && !g_interrupted) {
        mcvi::render(editor);
        char ch = mcvi::read_key();
        if (ch != 0) {
            mcvi::handle_key(editor, ch);
        }
    }

    return 0;
}
