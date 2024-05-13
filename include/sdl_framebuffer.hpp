#ifndef _SDL_FB_H_
#define _SDL_FB_H_

#include "fb_out.hpp"

#include <SDL2/SDL.h>

class FramebufferSDL : public IFramebufferOut
{
public:
    FramebufferSDL(const char *window_title);
    virtual ~FramebufferSDL();

    virtual void Clear() override;

    virtual int Put(int32_t x, int32_t y, bool val) override;

    virtual void Tick() override;

private:
    SDL_Window      *window_;
    SDL_Renderer    *renderer_;
    SDL_Surface     *window_surface_;

    int32_t window_width_;
    int32_t window_height_;

    bool __fb_array_[256 * 256];
};

#endif