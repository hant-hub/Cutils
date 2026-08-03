#define CU_IMPL
#define DS_IMPL
#include <core/cutils.h>
#include <ds/ds.h>



int main() {

    logfile.buffered = FALSE;

    Exar new_exar = InitExar(GlobalAllocator, int);
    for (u32 i = 0; i < 30; i++) {
        ExarInsert(&new_exar, &i);
    }

    printlog("Test Get\n");
    for (u32 i = 0; i < new_exar.size; i++) {
        int* v = ExarGet(&new_exar, i);
        assert(*v == i);
        printlog("\tGet %d at %d\n", *v, i);
    }

    ExarClear(&new_exar);

    for (u32 i = 0; i < 30; i++) {
        u32 val = i * 2;
        ExarInsert(&new_exar, &val);
    }

    printlog("Test Get\n");
    for (u32 i = 0; i < new_exar.size; i++) {
        int* v = ExarGet(&new_exar, i);
        assert(*v == i*2);
        printlog("\tGet %d at %d\n", *v, i);
    }


    ExarFree(&new_exar);

    Exar lex = InitExar(GlobalAllocator, i64);
    for (i64 i = 0; i < 30; i++) {
        i64 val = i - 15;
        ExarInsert(&lex, &val);
    }

    printlog("Test Get\n");
    for (i32 i = 0; i < lex.size; i++) {
        i64* v = ExarGet(&lex, i);
        assert(*v == i - 15);
        printlog("\tGet %d at %d\n", *v, i);
    }

    ExarFree(&lex);


}
