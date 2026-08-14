#define SB_IMPL
#include <sbuild/sbuild.h>

task_func(test_proc, t) {
    printf("%*s\n", t->name.len, t->name.data);
}

static file f;

task_func(fstart, t) {
    printf("hit\n");
    f = fileopen(sstring("/tmp/testfile"), FILE_READ | FILE_WRITE | FILE_CREAT | FILE_TRUNC);
}

task_func(fout, t) {
    filewrite(&f, sstring("test"));
}

task_func(fend, t) {
    fileclose(f);
}



int main(int argc, char* argv[]) {
    logfile.buffered = FALSE;
    TaskGraph g = CreateTaskGraph();
    
    //test proc
    {
        Task* t = AddTask(&g, "test");
        AddProc(t, test_proc);
        Exec(t, 1);
        ClearTaskGraph(&g);
    }

    //test linked procs
    {
        Task* start = AddTask(&g, "start");
        AddProc(start, fstart);

        Task* mid = AddTask(&g, "mid", start);
        AddProc(mid, fout);

        Task* end = AddTask(&g, "end", mid);
        AddProc(end, fend);

        Task* dummy = AddDummyTask(&g, end);

        PrintTaskTree(end, FALSE);
        Exec(dummy, 4);
        ClearTaskGraph(&g);
    }

    //test compile_cmd entry
    {
        _c_intern.compile_file = sstring("/tmp/compile_cmds.json");
        Task* start = AddTask(&g, "start");
        AddProc(start, beg_compile);

        Task* mid = AddTask(&g, "mid", start);
        AddProc(mid, add_entry);
        AddCmd(mid, "test/tests/sbuild/compile.c", "-I", "include/");

        Task* mid1 = AddTask(&g, "mid1", mid);
        AddProc(mid1, add_entry);
        AddCmd(mid1, "include/sbuild/sbuild.h", "-I", "include/");

        Task* end = AddTask(&g, "end", mid1);
        AddProc(end, end_compile);

        PrintTaskTree(end, FALSE);
        Exec(end, 4);
        ClearTaskGraph(&g);
    }

    //api test

    //enabled
    {
        BegCompile(&g, TRUE, sstring("/tmp/compile_cmds.json"));
        Task* t = CompileCmd(&g, "a.out", Spread("../resources/hello.c"));
        Task* t2 = CompileCmd(&g, "a.out", Spread("../resources/hello.c"));
        Task* end = AddDummyTask(&g, t, t2);
        EndCompile(&g, end);

        PrintTaskTree(end, FALSE);
        Exec(t, 1);
        ClearTaskGraph(&g);
    }

    //disabled
    {

        BegCompile(&g, FALSE, sstring(""));
        Task* t = CompileCmd(&g, "a.out", Spread("../resources/hello.c"));
        Task* t2 = CompileCmd(&g, "a.out", Spread( "../resources/hello.c" ), t);
        Task* end = AddDummyTask(&g, t, t2);

        EndCompile(&g, end);

        PrintTaskTree(end, FALSE);
        Exec(t, 1);
        ClearTaskGraph(&g);
    }

    FreeTaskGraph(&g);
}
