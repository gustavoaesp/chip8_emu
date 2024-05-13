#include "chip8.hpp"
#include "fb_out.hpp"
#include "sdl_framebuffer.hpp"

#include <SDL2/SDL.h>

#include <iostream>
#include <fstream>
#include <memory>
#include <vector>

/*
 * TODO Make this configurable
 */
static constexpr int kCyclesPerFrame = 32;
static constexpr int kFrameMillis = 16;

static constexpr int kTotalMemory = 0x1000;

int rom_read(const char* filename, uint8_t* mem) {
    std::ifstream infile(filename, std::ios::binary | std::ios::ate);
    if (!infile) {
        std::cerr << "Could not open rom \"" << filename << "\"\n";
        return -1;
    }

    size_t size = infile.tellg();
    infile.seekg(std::ios::beg);

    std::cout << "ROM size: " << size << "\n";

    if (size > (kTotalMemory - 0x200)) {
        std::cerr << "ROM too big\n";
        return -1;
    }

    infile.read((char*)mem + 0x200, size);

    return 0;
}

void sdl_keyevent(SDL_Event* event, uint8_t* keypad) {
    int pressed = (event->type == SDL_KEYDOWN);
    switch(event->key.keysym.sym) {
    case SDLK_1:
        keypad[1] = pressed;
        break;
    case SDLK_2:
        keypad[2] = pressed;
        break;
    case SDLK_3:
        keypad[3] = pressed;
        break;
    case SDLK_4:
        keypad[0xc] = pressed;
        break;
    /* Second row */
    case SDLK_q:
        keypad[4] = pressed;
        break;
    case SDLK_w:
        keypad[5] = pressed;
        break;
    case SDLK_e:
        keypad[6] = pressed;
        break;
    case SDLK_r:
        keypad[0xd] = pressed;
        break;
    /* Third row */
    case SDLK_a:
        keypad[7] = pressed;
        break;
    case SDLK_s:
        keypad[8] = pressed;
        break;
    case SDLK_d:
        keypad[9] = pressed;
        break;
    case SDLK_f:
        keypad[0xe] = pressed;
        break;
    /* Fourth row*/
    case SDLK_z:
        keypad[0xa] = pressed;
        break;
    case SDLK_x:
        keypad[0] = pressed;
        break;
    case SDLK_c:
        keypad[0xb] = pressed;
        break;
    case SDLK_v:
        keypad[0xf] = pressed;
        break;
    }
}

int main (int argc, char** argv)
{
    bool running = true;
    std::vector<uint8_t> mem(kTotalMemory);
    uint8_t keypad[16] = {};
    uint8_t timer = 0;
    uint8_t stimer = 0;
    Chip8_CPU chip8_cpu;

    /* TODO handle this with getopt */
    if (argc > 1) {
        std::cout << "Reading rom \"" << argv[1] << "\"\n";
        if (rom_read(argv[1], mem.data())) {
            return -1;
        }
    } else {
        std::cerr << "Usage: chip8_emu [[rom_name]]\n";
        return -1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL\n";
        return -1;
    }

    {
        std::unique_ptr<IFramebufferOut> fb_out = std::make_unique<FramebufferSDL>(
            "Chip8 VM by Gustavo A Espinoza"
        );
        fb_out->Clear();

        while (running) {
            int64_t start, end;
            int64_t delay_time;
            SDL_Event e;

            start = SDL_GetTicks64();

            for (int cycle = 0; cycle < kCyclesPerFrame; ++cycle) {
                if(chip8_cpu.Tick(fb_out.get(), mem.data(), keypad, &timer, &stimer)) {
                    /* TODO dump registers and stack values on error */
                    std::cerr << "Chip8 tick error\n";
                    fb_out.reset();
                    SDL_Quit();
                    return -1;
                }
            }

            while (SDL_PollEvent(&e) > 0) {
                switch(e.type)
                {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    sdl_keyevent(&e, keypad);
                    switch (e.key.keysym.sym) {
                    case SDLK_RETURN:
                        /* RESET */
                        memset(mem.data(), kTotalMemory, 0);
                        chip8_cpu.Reset();
                        fb_out->Clear();
                        rom_read(argv[1], mem.data());
                        break;
                    }
                    break;
                }
            }
            fb_out->Tick();
            end = SDL_GetTicks64();
            delay_time = kFrameMillis - (start - end);
            if (delay_time >= 0) {
                SDL_Delay(delay_time);
                if (timer > 0) { timer--; } else { timer = 0; }
                if (stimer > 0) { stimer--; } else { stimer = 0; }
            }
        }
    }

    SDL_Quit();

    return 0;
}