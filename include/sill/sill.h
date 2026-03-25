#ifndef SILL_H
#define SILL_H

#include <core/cutils.h>

typedef enum SILLformat {
    SILL_RGB_8 = 0,
} SILLformat;

void PPMoutput(file* f, void* pixelData, SILLformat format, u32 width, u32 height);
void PAMoutput(file* f, void* pixelData, SILLformat format, u32 width, u32 height);

void* BMPread(ArenaAllocator a, file* f, u32* width, u32* height);

#endif
