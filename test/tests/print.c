#include <cutils.h>
#include <stdio.h>

int main() {
    char buf[4097] = {0};

    sformat((SString){.data = (i8 *)buf, .len = 4096},
            "hit \"%s\" \"%n\" \"%d\", \"%f\"", sstring("bam"), "slam",
            1005432100, 0.02124);

    printf("sformat: %s\n", buf);

    fformat(
        &(file){
            .handle = 1,
        },
        "fformat: hit \"%s\" \"%n\" \"%d\", \"%f\" \"%c\" \"%x\" \"%u\"\n", sstring("bam"), "slam",
        1005432100, 0.02124, 'a', &buf, -1);

    printlog("test: log\n");
    printwarn("test: warn\n");
    printerr("test: err\n");

    printlog("test: %f\n", 0.0);

    debuglog("test");
    debugwarn("test");
    debugerr("test");

    debuglog("%d", 0);
}
