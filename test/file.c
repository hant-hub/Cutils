#include <cutils.h>
#include <stdio.h>

int main() {

    file t = fileopen(sstring("/proc/self/status"), FILE_READ);
    file out = {
        .handle = 1,
        .buffered = 1,
        .closefd = 1,
    };

    i8 buf[100] = {0};
    SString dst = {
        .len = sizeof(buf),
        .data = buf,
    };

    fileread(dst, t);

    printf("file ----------\n");
    filewrite(&out, dst);
    fileflush(&out);
    printf("\nfile ----------\n");

    fileclose(t);
    fileclose(out);
}
