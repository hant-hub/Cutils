#include <sill/sill.h>

struct pixel {
    char r, g, b;
};

struct pixel b = {0};
struct pixel w = {.r = 255, .b = 0, .g=255};



int main() {
    struct pixel data[] = {
        b, b, b, b, b, b, b, b,
        b, b, w, b, b, w, b, b,
        b, b, w, b, b, w, b, b,
        b, b, w, b, b, w, b, b,
        b, b, b, b, b, b, b, b,
        b, b, w, b, b, w, b, b,
        b, b, b, w, w, b, b, b,
        b, b, b, b, b, b, b, b,
    };

    file f = fileopen(sstring("test.pam"), FILE_WRITE | FILE_CREAT | FILE_TRUNC); 
    PAMoutput(&f, data, SILL_RGB_8, 8, 8);
    fileclose(f);
    return 0;
}
