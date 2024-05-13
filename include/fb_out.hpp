#ifndef _FB_OUT_H_
#define _FB_OUT_H_

#include <cstdint>

static constexpr int32_t kNativeWidth = 64;
static constexpr int32_t kNativeHeight = 32;

class IFramebufferOut
{
public:
    virtual ~IFramebufferOut() {}

    virtual void Clear() = 0;

    virtual int Put(int32_t x, int32_t y, bool val) = 0;

    virtual void Tick() = 0;

protected:
};

#endif