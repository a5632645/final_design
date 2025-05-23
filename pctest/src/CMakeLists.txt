include(usflib/include/single_header.cmake)

file(GLOB_RECURSE SRCS
    "Waveguide/*.cpp"
    "Waveguide/*.c"
)
add_executable(Waveguide ${SRCS})
target_link_libraries(Waveguide PRIVATE raylib rtmidi)
target_include_directories(Waveguide PRIVATE usflib/include Waveguide)
add_compile_options(-Wall -Wextra -Wpedantic)