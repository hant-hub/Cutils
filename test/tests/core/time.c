#include <core/cutils.h>

int main() {

    logfile->buffered = FALSE;
    time_spec a = {
        .s = 1,
        .ns = 0,
    };

    time_spec b = {
        .s = 2,
        .ns = 0,
    };

    time_spec c = {
        .s = 1,
        .ns = 1,
    };

    assert(timecmp(a, <, b));
    assert(timecmp(a, <=, b));
    assert(!timecmp(a, >, b));
    assert(!timecmp(a, >=, b));
    assert(!timecmp(a, ==, b));

    assert(timecmp(a, <, c));
    assert(timecmp(a, <=, c));
    assert(!timecmp(a, >, c));
    assert(!timecmp(a, >=, c));
    assert(!timecmp(a, ==, c));

    assert(!timecmp(a, <, a));
    assert(timecmp(a, <=, a));
    assert(!timecmp(a, >, a));
    assert(timecmp(a, >=, a));
    assert(timecmp(a, ==, a));



    a = (time_spec){ 1, 2 };
    b = (time_spec){ 2, 4 };

    assert(timecmp(time_sum(a, b), ==, ((time_spec){ 3, 6 })));
    assert(timecmp(time_diff(a, b), ==, ((time_spec){ 1, 2 })));


    //overflow sum
    a = (time_spec){ 0, 800000000 };
    b = (time_spec){ 0, 200000000 };

    assert(timecmp(time_sum(a, b), ==, ((time_spec){ 1, 0 })));

    //overflow diff
    a = (time_spec){ 1, 800000000 };
    b = (time_spec){ 2, 000000000 };

    assert(timecmp(time_diff(a, b), ==, ((time_spec){ 0, 200000000 })));

    //negative diff (if the duration would be negative we force 0)
    assert(timecmp(time_diff(b, a), ==, ((time_spec){ 0, 0 })));
}
