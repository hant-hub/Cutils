#include <core/cutils.h>

#define STRBASE_IMPL
#include <strbase/strbase.h>

int main() {
    StrBase* data = &(StrBase){GlobalAllocator};

    StrID test = StrBaseAdd(data, sstring("test"));
    StrID test2 = StrBaseAdd(data, sstring("test2"));
    StrBaseAdd(data, sstring("test3"));
    StrBaseAdd(data, sstring("test4"));
    StrBaseAdd(data, sstring("test5"));
    StrBaseAdd(data, sstring("test6"));
    StrBaseAdd(data, sstring("test7"));
    StrBaseAdd(data, sstring("test8"));


    StrID i = StrBaseGet(data, sstring("test"));
    assert(i == test);
    i = StrBaseGet(data, sstring("test2"));
    assert(i == test2);
    i = StrBaseGet(data, sstring("invalid"));
    assert(i == -1);

}
