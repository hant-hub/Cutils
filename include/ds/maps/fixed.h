#ifndef FIXED_MAP_H
#define FIXED_MAP_H
#include <core/cutils.h>

//Helpers and functions not dependent on key/value types

typedef enum flat_imap_meta: u8 {
    META_EMPTY    = 0b10000000,
    META_DEL      = 0b11111110,
    META_FULL     = 0b01111111,
    META_SENTINEL = 0b11111111,
} flat_imap_meta;

static inline u64 getH1(u64 h) {
    return (h >> 7);
}

static inline u8 getH2(u64 h) {
    return (h & META_FULL);
}

//gets overflow bit
static inline u8 getH3(u64 h) {
    return h % 8;
}

static const u64 spread = 0x0101010101010101;
static const u64 final_mask = 0x8080808080808080;

static inline u64 getMask(u64 meta, u64 mask) {
    const u64 matches = meta ^ mask;
    u64 res = (matches - spread) & ~matches;
    return res & final_mask;
}


#endif
