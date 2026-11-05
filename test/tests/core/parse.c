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

    u32 i;
    d = StrtoU32(sstring("0"), &i);
    assert(d == 1);
    assert(i == 0);

    d = StrtoU32(sstring("1"), &i);
    assert(d == 1);
    assert(i == 1);

    d = StrtoU32(sstring("10"), &i);
    assert(d == 2);
    assert(i == 10);

    d = StrtoU32(sstring("100"), &i);
    assert(d == 3);
    assert(i == 100);

    d = StrtoU32(sstring("240"), &i);
    assert(d == 3);
    assert(i == 240);

    d = StrtoU32(sstring("240L"), &i);
    assert(d == 3);
    assert(i == 240);

    printlog("Success!\n");

}
