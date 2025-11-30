#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "Global.h"

struct FrameBuffer {
    std::atomic<bool> ready;
    UINT32 width;
    UINT32 height;
    GUID format;
    BYTE* data[Global::maxFrameDataSize];
};

#endif //FRAMEBUFFER_H
