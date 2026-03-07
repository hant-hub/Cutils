#include <core/cutils.h>


int main() {

    u32 data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    slice(u32) s = {0};
    //3 ... 6
    sliceAssign(s, data, 2, 5);

    for (u32 i = 0; i < s.size; i++) {
        assert(s.data[i] == data[i+2]);
    }

    return 0;
}
