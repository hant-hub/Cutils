#define SILL_IMPL
#include <sill/sill.h>


int main() {

    ScratchArena sc = ScratchArenaGet(NULL);

    file f = fileopen(sstring("../resources/kermit.bmp"), FILE_READ); 
    u32 width, height;
    void* data = BMPread(sc.arena, &f, &width, &height);
    fileclose(f);
    
    f = fileopen(sstring("bmp_test.ppm"), FILE_WRITE | FILE_CREAT | FILE_TRUNC);

    PPMoutput(&f, data, SILL_RGB_8, width, height);

    fileclose(f);
    ScratchArenaEnd(sc);
    return 0;
}
