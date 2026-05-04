#include <csignal>

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

    mcvi::RawTerminal terminal;
    mcvi::Editor editor;
    if (argc > 1) {
        mcvi::load_initial_file(editor, argv[1]);
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
