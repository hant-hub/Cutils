#include <cutils.h>
#include <stdio.h>


int main() {
    char buf[4097] = {0};

    //sformat((SString){.data = (i8*)buf, .len = 4096},
    //        "hit \"%s\" \"%n\" \"%d\"", sstring("bam"), "slam", 1005432100);

    sformat((SString){.data = (i8*)buf, .len = 4096},
            "float: \"%f\"", 0.02124);

    printf("%s\n", buf);
    printf("%f\n", 0.2124);

}
