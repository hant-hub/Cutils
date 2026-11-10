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

    //handle negative
    d = Strtofloat(sstring("-1.0"), &f);
    assert(d == 4);
    assert(f == -1.0);

    //Integer

    u32 u;
    d = StrtoU32(sstring("0"), &u);
    assert(d == 1);
    assert(u == 0);

    d = StrtoU32(sstring("1"), &u);
    assert(d == 1);
    assert(u == 1);

    d = StrtoU32(sstring("10"), &u);
    assert(d == 2);
    assert(u == 10);

    d = StrtoU32(sstring("100"), &u);
    assert(d == 3);
    assert(u == 100);

    d = StrtoU32(sstring("240"), &u);
    assert(d == 3);
    assert(u == 240);

    d = StrtoU32(sstring("240L"), &u);
    assert(d == 3);
    assert(u == 240);


    i32 i;
    d = StrtoI32(sstring("0"), &i);
    assert(d == 1);
    assert(i == 0);

    d = StrtoI32(sstring("1"), &i);
    assert(d == 1);
    assert(i == 1);

    d = StrtoI32(sstring("10"), &i);
    assert(d == 2);
    assert(i == 10);

    d = StrtoI32(sstring("100"), &i);
    assert(d == 3);
    assert(i == 100);

    d = StrtoI32(sstring("240"), &i);
    assert(d == 3);
    assert(i == 240);

    d = StrtoI32(sstring("240L"), &i);
    assert(d == 3);
    assert(i == 240);

    d = StrtoI32(sstring("-1"), &i);
    assert(d == 2);
    assert(i == -1);

    printlog("Success!\n");

}
