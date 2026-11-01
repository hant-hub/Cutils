#include <core/cutils.h>
#include <stdio.h>

int main() {
    logfile->buffered = FALSE;
    setdirExe();

    //basic functionality
    float f;
    u32 d = Strtofloat(sstring("1.0"), &f);

    assert(d == 3);
    assert(f == 1.0);

    //test fractional
    d = Strtofloat(sstring("1.5"), &f);

    assert(d == 3);
    assert(f == 1.5);

    //zero
    d = Strtofloat(sstring("0.0"), &f);

    assert(d == 3);
    assert(f == 0.0);

    //integer
    d = Strtofloat(sstring("5"), &f);

    assert(d == 1);
    assert(f == 5.0);

    //omit leading zero
    d = Strtofloat(sstring(".5"), &f);

    assert(d == 2);
    assert(f == .5);

    //stop on non numerical
    d = Strtofloat(sstring(".5d"), &f);

    assert(d == 2);
    assert(f == .5);

    printlog("Success!\n");
}
