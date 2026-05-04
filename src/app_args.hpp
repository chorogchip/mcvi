#pragma once

#include <string>

namespace mcvi {

struct AppArgs {
    std::string filename;
    std::string auto_input;
    bool auto_mode = false;
    bool help = false;
};

void print_usage(const char* program);
AppArgs parse_args(int argc, char** argv);
int run_auto(const AppArgs& args);

} // namespace mcvi
