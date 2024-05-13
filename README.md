# Simple chip8 virtual machine using SDL2
A simple chip8 VM implementation I made in my free time.

## Build
Just create a build folder and build with cmake
```	bash
# in the project folder
mdkir build && cd build
cmake ..
make
```

**Note:** The SDL2 library is required to be installed in the system.

## Running
Just run the resulting elf with a rom file
```bash
./chip8_emu some_rom_file
```