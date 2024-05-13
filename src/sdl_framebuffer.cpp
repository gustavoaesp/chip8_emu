#include "sdl_framebuffer.hpp"

#include <iostream>

static inline int get_fb_array(int x, int y) {
    return (y * kNativeWidth) + x;
}

FramebufferSDL::FramebufferSDL(const char *window_title):
    window_width_(640), window_height_(320), __fb_array_{}
{
    window_ = SDL_CreateWindow(
        window_title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        window_width_, window_height_,
    0);
    if (!window_) {
        std::cerr << "Failed to create window\n";
        return;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) {
        std::cerr << "Failed to create renderer\n";
        return;
    }
}

FramebufferSDL::~FramebufferSDL()
{
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
}

void FramebufferSDL::Clear()
{
    memset(__fb_array_, 0, sizeof(__fb_array_));
}

int FramebufferSDL::Put(int32_t x, int32_t y, bool value)
{
    if (!(x < kNativeWidth || y < kNativeHeight)) {
        return 0;
    }
    if (x < 0 || y < 0) {
        return 0;
    }

    __fb_array_[get_fb_array(x, y)] ^= 1;

    return __fb_array_[get_fb_array(x, y)] ^ 1;
}

void FramebufferSDL::Tick()
{
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 0xff);
    SDL_RenderClear(renderer_);

    int32_t w_proportion = window_width_ / kNativeWidth;
    int32_t h_proportion = window_height_ / kNativeHeight;

    SDL_SetRenderDrawColor(renderer_, 0xff, 0xff, 0xff, 0xff);

    for (int y = 0; y < kNativeHeight; ++y) {
        for (int x = 0; x < kNativeWidth; ++x) {
            SDL_Rect rect{
                .x = x * w_proportion,
                .y = y * h_proportion,
                .w = w_proportion,
                .h = h_proportion
            };

            if (__fb_array_[get_fb_array(x, y)]) {
                SDL_RenderFillRect(renderer_, &rect);
            }
        }
    }
    SDL_RenderPresent(renderer_);
}