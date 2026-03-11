#ifndef GA_H
#define GA_H

//Currently we assume AVX/SIMD

#include <immintrin.h>

typedef struct GAPlane {
    __m128 p; //32 x 4 (ae1 + be2 + ce3 + de0 = 0)
              //ax + by + cz + d = 0
} GAPlane;

typedef struct GALine {
} GALine;

typedef struct GAPoint {
    __m128 p; //32 x 4 floats
} GAPoint;




#ifdef GA_IMPL



#endif
#endif
