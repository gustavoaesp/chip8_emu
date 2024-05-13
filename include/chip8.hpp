#ifndef _CHIP_8_H_
#define _CHIP_8_H_

#include "fb_out.hpp"

#include <cstdint>

class Chip8_CPU
{
public:
    Chip8_CPU();
    ~Chip8_CPU();

    void Reset();

    int Tick(
        IFramebufferOut *fb_out,
        uint8_t *mem,
        uint8_t *keypad,
        uint8_t *timer,
        uint8_t *stimer
    );

private:
    uint8_t V[16];
    uint16_t I, pc;
    uint8_t sp;

    uint16_t stack[16];

    uint8_t rand_count;
};

#endif