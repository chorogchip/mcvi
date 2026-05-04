cmake_minimum_required(VERSION 3.16)

project(mcvi LANGUAGES CXX)

find_package(ZLIB REQUIRED)

add_executable(mcvi
    src/main.cpp
    src/app_args.cpp
    src/mode.cpp
    src/block_alias.cpp
    src/direction.cpp
    src/editor.cpp
    src/terminal.cpp
    src/renderer.cpp
    src/command.cpp
    src/input.cpp
    src/world_io.cpp
    src/world_io_common.cpp
    src/world_io_json.cpp
    src/nbt_writer.cpp
    src/world_io_schem.cpp
    src/world_io_nbt.cpp
    src/world_io_mca.cpp
)

target_compile_features(mcvi PRIVATE cxx_std_17)
set_target_properties(mcvi PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_compile_options(mcvi PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

target_link_libraries(mcvi PRIVATE ZLIB::ZLIB)
