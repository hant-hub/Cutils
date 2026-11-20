#define SB_IMPL
#include "./sbuild_st.h"

#define OPTS                                                                   \
    X(f, bool)                                                                 \
    X(t, string)                                                               \
    X(j, int)                                                                  \
    X(no_debug, bool)                                                          \
    X(p, bool)                                                                 \
    X(o, int)                                                                  \
    X(m, int)                                                                  \
    X(e, bool)

#include "sa.h"

const char* optimization_levels[] = {
    "", "-O0", "-O1", "-O2", "-O3",
};

int main(int argc, char* argv[]) {
    logfile->buffered = FALSE;

    AutoRebuild(argc, argv);
    setdirExe();

    SA options = Parse(argc, argv);
    if (options.err)
        exit(-1);
    SAvals opts = options.vals;

    //-o=1 -> -O1 while -o=0 -> -O0 and all other cases -> ""
    u32 opt_level = opts.o.set && opts.o.val <= 3 ? opts.o.val + 1 : 0;
    u32 num_jobs = opts.j.set ? opts.j.val : 4;

    const char* march = opts.m.set ? "-march=native" : "";
    const char* p_asm = opts.e.set ? "-S" : "";
    const char* debug_flag = opts.no_debug.set ? "" : "-g";

    printlog("Build Options: O=%d\n", opt_level - 1);

    TaskGraph g = CreateTaskGraph();
    ScratchArena sc = ScratchArenaGet(NULL);

    Task* pre_compile = AddDummyTask(&g);
    {
        Task* top = NULL;
        if (!filestat(sstring("build")).stats.size) {
            Task* mkdir = AddTask(&g, "Make Build Dir");
            AddCmd(mkdir, "mkdir", "build/");
            top = mkdir;
        }

        if (!filestat(sstring("build/tests")).stats.size) {
            Task* mkdir = AddTask(&g, "Make Test Dir", top);
            AddCmd(mkdir, "mkdir", "build/tests");
            top = mkdir;
        }
        
        AddDep(pre_compile, top);
    }

    if (opts.f.set) {
        Task* format = AddDummyTask(&g);
        DirBeginRec(sstring("./"), 10);
        DirType t = T_UNKNOWN;
        while (t != T_END) {
            SString name = DirNextRec(&t);
            char* n = ArenaAllocZero(sc.arena, name.len + 1);
            memcpy(n, name.data, name.len);

            if (!Sstrcmp(filepathext(name), sstring(".c")) &&
                !Sstrcmp(filepathext(name), sstring(".h"))) {
                continue;
            }

            printlog("format: %n\n", n);
            Task* format_file = AddTask(&g, "format file");
            AddCmd(format_file, "clang-format", "-i", n);
            AddDep(format, format_file);
        }
        AddDep(pre_compile, format);
    }

    // exec and complete pre compile steps
    Exec(pre_compile, num_jobs);
    ClearTaskGraph(&g);

    BegCompile(&g, TRUE, sstring("./compile_commands.json"));

    // static lib
    Task* static_lib = AddDummyTask(&g);
    {
        Task* compile = CompileCmd(&g, "./build/cutils.o",
                                   Spread("-c", debug_flag, "./src/cutils.c",
                                          "-lm", "-Iinclude/", "-Wall",
                                          optimization_levels[opt_level], march));

        static_lib = AddTask(&g, "archive Static Lib", compile);
        AddCmd(static_lib, "ar", "rcs", "build/cutils.a", "build/cutils.o");
        //AddCmd(static_lib, "echo", "test");
    }

    // cp resources
    Task* resource = AddTask(&g, "resource copy");
    AddCmd(resource, "cp", "-r", "resources", "build/");

    // gather tests
    dynArray(SString) groups = {GlobalAllocator};

    DirBegin(sstring("test/tests"));
    DirType t = T_UNKNOWN;
    while (t != T_END) {
        SString name = DirNext(&t);
        if (t != T_DIR)
            continue; // skip non directories
        if (Sstrcmp(name, sstring(".")))
            continue;
        if (Sstrcmp(name, sstring("..")))
            continue;

        i8* n = ArenaAllocZero(sc.arena, name.len + 1);
        memcpy(n, name.data, name.len);
        dynPush(groups, ((SString){name.len, n}));
    }

    // build tests
    const SString src_prefix = sstring("./test/tests");
    const SString dst_prefix = sstring("./build/tests");

    Task** tests = ArenaAllocZero(sc.arena, sizeof(Task*) * groups.size);

    for (u32 i = 0; i < groups.size; i++) {
        ScratchArena sc2 = ScratchArenaGet(sc.arena);

        u32 group_len = groups.data[i].len + src_prefix.len + 1;
        i8* group = ArenaAllocZero(sc.arena, group_len + 1);
        sformat((SString){group_len, (i8*)group}, "%s/%s", src_prefix,
                groups.data[i]);

        tests[i] = AddDummyTask(&g);

        DirType t = T_UNKNOWN;
        DirBegin((SString){group_len, group});

        while (t != T_END) {
            SString file = DirNext(&t);
            if (t != T_FILE)
                continue;

            u32 dst_len = groups.data[i].len + dst_prefix.len + file.len +
                          2; // 2 = num of additional '/'
            i8* dst = ArenaAllocZero(sc.arena, dst_len + 1);
            sformat((SString){dst_len, (i8*)dst}, "%s/%s", dst_prefix,
                    groups.data[i]);

            Task* mkdir = AddTaskCond(
                !filestat((SString){strlen((char*)dst), dst}).stats.size, &g,
                "mkdir test");
            AddCmd(mkdir, "mkdir", "-p", (char*)dst);

            sformat((SString){dst_len, (i8*)dst}, "%s/%s/%s", dst_prefix,
                    groups.data[i], filepathbasename(file));

            u32 src_len = groups.data[i].len + src_prefix.len + file.len + 2;
            i8* src = ArenaAllocZero(sc.arena, src_len + 1);
            sformat((SString){src_len, (i8*)src}, "%s/%s/%s", src_prefix,
                    groups.data[i], file);

            // tests are always on default optimization with debugging info on
            Task* compile = CompileCmd(
                &g, (char*)dst,
                Spread((char*)src, "build/cutils.a", "-Iinclude/", "-Wall", "-g", "-lm", p_asm, march),
                mkdir, static_lib);

            AddDep(tests[i], compile);
        }

        ScratchArenaEnd(sc2);
    }

    // build test runner
    Task* runner = CompileCmd(
        &g, "./build/runner",
        Spread("test/runner.c", "build/cutils.a", "-Iinclude/", "-g", "-lm"),
        static_lib);

    Task* end = AddDummyTask(&g, resource, static_lib);

    for (u32 i = 0; i < groups.size; i++) { AddDep(end, tests[i]); }
    end = EndCompile(&g, end);

    if (opts.t.set) {
        AddDep(end, runner);
        Task* run_test = AddTask(&g, "run_test", end);
        if (strcmp(opts.t.val, "all") == 0 || strcmp(opts.t.val, "") == 0) {
            AddCmd(run_test, "./build/runner");
        } else {
            AddCmd(run_test, "./build/runner", "v", opts.t.val);
        }

        end = run_test;
    }

    if (opts.p.set) {
        PrintTaskTree(end, FALSE);
    } else {
        Exec(end, num_jobs);
    }

    dynFree(groups);
    ScratchArenaEnd(sc);
    FreeTaskGraph(&g);
}
