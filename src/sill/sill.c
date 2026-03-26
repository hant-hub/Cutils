#include <core/cutils.h>
#include <sill/sill.h>

static u32 Format_to_depth[] = {
    [SILL_RGB_8] = 3,
    [SILL_RGBA_8] = 4,
};

static SString Format_to_tuple[] = {
    [SILL_RGB_8] = sstring("RGB"),
    [SILL_RGBA_8] = sstring("RGB_ALPHA"),
};

void PPMoutput(file* f, void* pixelData, SILLformat format, u32 width, u32 height) {
    fformat(f, "P6\n%d %d\n255\n", width, height);

    i8* data = pixelData;
    u32 channels = Format_to_depth[format];
    debuglog("Size (%d %d)", width, height);
    debuglog("Channels: %d", channels);

    bool8 prev_buffered = f->buffered;
    f->buffered = TRUE;

    u32 cursor = 0;
    for (u32 i = 0; i < width * height; i++) {
        filewrite(f, (SString){.len = 3, .data = &data[cursor]});
        cursor += channels;
    }

    fileflush(f);
    f->buffered = prev_buffered;
}

void PAMoutput(file* f, void* pixelData, SILLformat format, u32 width, u32 height) {
    u32 depth = Format_to_depth[format];

    fformat(f, "P7\n");
    fformat(f, "WIDTH %d\n", width);
    fformat(f, "HEIGHT %d\n", height);
    fformat(f, "DEPTH %d\n", Format_to_depth[format]);
    fformat(f, "MAXVAL %d\n", 255);
    fformat(f, "TUPLTYPE %s\n", Format_to_tuple[format]);
    fformat(f, "ENDHDR\n");

    filewrite(f, (SString){.data = (i8*)pixelData, .len = width * height * Format_to_depth[format]});
}


typedef struct parsingState {
    SString data;
    u32 idx;
} parsingState;

#define expect(p, c) \
    do { \
        if (p->data.data[p->idx] != c) { \
            panic(); \
        } \
        p->idx++; \
    } while (0);

static u32 ReadU32(parsingState* p) {
    u32 val = *(u32*)&p->data.data[p->idx];
    p->idx += 4;
    return val;
}

static i32 ReadI32(parsingState* p) {
    i32 val = *(i32*)&p->data.data[p->idx];
    p->idx += 4;
    return val;
}

static u16 ReadU16(parsingState* p) {
    u16 val = *(u32*)&p->data.data[p->idx];
    p->idx += 2;
    return val;
}

void* BMPread(ArenaAllocator a, file* f, u32* outwidth, u32* outheight, u32* outchannels) {
    ScratchArena sc = ScratchArenaGet(a);    
    parsingState* state = &(parsingState){
        .data = {
            .data = ArenaAlloc(&sc.arena, f->stats.size),
            .len = f->stats.size,
        }
    };

    fileread(state->data, *f);
    expect(state, 'B');
    expect(state, 'M');

    u32 size = ReadU32(state);
    ReadU32(state);
    u32 offset = ReadU32(state);
    u32 infosize = ReadU32(state);
    i32 width = ReadI32(state);
    i32 height = ReadI32(state);
    u16 planes = ReadU16(state);

    debuglog("Image Dimensions (%d %d) %d", width, height, planes);
    
    u16 pixelSize = ReadU16(state);
    debuglog("Pixel Bits: %d", pixelSize);

    SString compressionTypes[] = {
        [0] = sstring("No Compression"),
        [1] = sstring("8 bit RLE"),
        [2] = sstring("4 bit RLE"),
    };

    u32 compression = ReadU32(state);
    debuglog("Compression: %s", compressionTypes[compression]);

    u32 compressedSize = ReadU32(state); //may be zero if no compression
    debuglog("Compressed Size: %d", compressedSize);

    u32 hresolution = ReadU32(state);
    u32 vresolution = ReadU32(state);

    debuglog("Resolution: (%d %d)", hresolution, vresolution);

    u32 UsedColors = ReadU32(state);
    u32 ImportantColors = ReadU32(state);
    debuglog("Used Colors/Important Colors: %d / %d", UsedColors, ImportantColors);

    if (pixelSize <= 8) {
        //parse color table
    }
    
    u32 linepadding = 4 - ((width * 3) % 4);
    debuglog("Line Padding: %d", linepadding); 

    u32 channels = *outchannels ? *outchannels : 3;

    debuglog("channels: %d", channels);
    u8* data = ArenaAlloc(&a, width * height * channels);    
    u32 cursor = (width * height * channels) - (width * channels);
    for (u32 i = 0; i < height; i++) {
        for (u32 i = 0; i < width; i++) {
            data[cursor + i*channels + 0] = state->data.data[state->idx + i*3 + 2];
            data[cursor + i*channels + 1] = state->data.data[state->idx + i*3 + 1];
            data[cursor + i*channels + 2] = state->data.data[state->idx + i*3 + 0];
            for (u32 j = 3; j < channels; j++) {
                data[cursor + i*channels + j] = 255; //default OPAQUE
            }
        }
        state->idx += (width * 3) + linepadding;
        cursor -= (width * channels);
    }

    ScratchArenaEnd(sc);

    *outwidth = width;
    *outheight = height;
    if (*outchannels == 0) {
        *outchannels = channels;
    }

    fileflush(&logfile);
    return data;
}

