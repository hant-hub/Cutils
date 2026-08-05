#include <core/cutils.h>

int main() {

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
}
