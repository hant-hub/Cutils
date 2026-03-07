#include <core/cutils.h>
#include <alloca.h>
#include <stdio.h>

int main() {
    setdirExe();
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



    SString data = sstring("Lorem ipsum, your momma");
    SString load = {
        .len = data.len,
        .data = alloca(data.len),
    };

    filesave(sstring("test"), data);
    fileload(load, sstring("test"));

    fformat(&logfile, "load: %s\n", load);

    filedelete(sstring("test"));

    assert(Sstrcmp(load, data));

    //string stuff

    SString filepath = sstring("a/b/c.lol");
    SString pop = sstring("a/b");

    assert(Sstrcmp(filepathpopdir(filepath), pop));
    assert(Sstrcmp(filepathpopdir(sstring("a")), sstring("a")));

    assert(Sstrcmp(filepathbasename(filepath), sstring("c.lol")));

    assert(Sstrcmp(filepathext(filepath), sstring(".lol")));
    assert(Sstrcmp(filepathext(sstring("somefile")), sstring("")));

    assert(filepathdepth(filepath) == 2);
    assert(filepathdepth(sstring("a/b/c/d")) == 3);

    //Directory

    DirBegin(sstring("."));

    debuglog("Directory Search: .");
    DirType dt;
    SString s;
    while ((s = DirNext(&dt), dt != T_END)) {
        debuglog("Directory: %s", s);

    }

    //Directory Recursive

    DirBeginRec(sstring("."), 1);

    debuglog("Recursive Directory Search: .");
    DirNextRec(&dt);
    while ((s = DirNextRec(&dt), dt != T_END)) {
        debuglog("Directory: %s", s);

    }

    //Directory

    DirBegin(sstring("fake"));

    debuglog("Directory Search: .");
    while ((s = DirNext(&dt), dt != T_END)) {
        debuglog("Directory: %s", s);

    }

    //Directory Recursive

    DirBeginRec(sstring("fake"), 2);

    debuglog("Recursive Directory Search: .");
    DirNextRec(&dt);
    while ((s = DirNextRec(&dt), dt != T_END)) {
        debuglog("Directory: %s", s);

    }

}
