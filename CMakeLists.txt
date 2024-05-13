project(chip8_emu)
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

set (CHIP8_EMU_SRC
    src/main.cpp
    src/chip8.cpp
    src/sdl_framebuffer.cpp
)

add_executable(chip8_emu ${CHIP8_EMU_SRC})
target_include_directories(chip8_emu PUBLIC include)
target_link_libraries(chip8_emu SDL2)