#define SILL_IMPL
#include <sill/sill.h>


int main() {

    ScratchArena sc = ScratchArenaGet(NULL);

    file f = fileopen(sstring("../resources/kermit.bmp"), FILE_READ); 
    u32 width, height;
    u32 channels = 4;
    void* data = BMPread(sc.arena, &f, &width, &height, &channels);
    fileclose(f);
    
    f = fileopen(sstring("bmp_test.pam"), FILE_WRITE | FILE_CREAT | FILE_TRUNC);

    PAMoutput(&f, data, SILL_RGBA_8, width, height);
    //PPMoutput(&f, data, SILL_RGBA_8, width, height);

    fileclose(f);
    ScratchArenaEnd(sc);
    return 0;
}
