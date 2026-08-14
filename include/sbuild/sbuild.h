/*
    Important!!!
    Must compile with -I set to include
*/



#ifndef SB_H
#define SB_H
#include <core/cutils.h>

#ifdef SB_IMPL
#define DS_IMPL
#endif
#include <ds/ds.h>

typedef struct TaskGraph TaskGraph;

struct Task;

#define task_func(name, task_arg) \
    void (name)(struct Task* task_arg)

typedef struct Task {
    TaskGraph* g; // back ref
    SString name; // for debug

    bool8 visited;
    u32 blockers;

    dynArray(struct Task*) pre;  // things this task depends on
    dynArray(struct Task*) post; // things which depend on this task
    
    task_func(*func, t);         //proc for io tasks
     
    dynArray(u32) cmds;          // command which makes up this task
} Task;

typedef struct TaskGraph {
    Exar tasks;
    dynArray(char) str_data;

    struct {
        dynArray(int) processes;
        dynArray(Task*) tasks;
    } running;
} TaskGraph;

typedef dynArray(i8) CBuf;

#define AutoRebuild(argc, argv, ...)                                           \
    _AutoRebuild(__FILE__, argc, argv,                                         \
                 ARRAY_SIZE(((const char*[]){__VA_ARGS__})), ##__VA_ARGS__)

#define AddDummyTask(g, ...) \
    _AddTask(g, "", ARRAY_SIZE(((Task*[]){__VA_ARGS__})),##__VA_ARGS__)


#define AddTask(g, name, ...)                                                  \
    _AddTask(g, name, ARRAY_SIZE(((Task*[]){__VA_ARGS__})), ##__VA_ARGS__)

#define AddTaskCond(cond, g, name, ...)                                        \
    (cond ? AddTask(g, name, ##__VA_ARGS__) : NULL)

#define AddCmd(t, ...)                                                         \
    _AddCmd(t, ARRAY_SIZE(((const char*[]){__VA_ARGS__})), ##__VA_ARGS__)

#define AddProc(t, p) \
    (t->func = p)

#define CompileCmd(g, file, c, ...) \
        Compile(g, file, ARRAY_SIZE((((const char*[]){c}))), (const char*[]){c}, ARRAY_SIZE(((Task*[]){__VA_ARGS__})),##__VA_ARGS__)

TaskGraph CreateTaskGraph();
void ClearTaskGraph(TaskGraph* g);
void FreeTaskGraph(TaskGraph* g);
Task* _AddTask(TaskGraph* g, const char* name, u32 num_dep, ...);
void _AddCmd(Task* t, u32 num_args, ...);

void AddDep(Task* t, Task* d);
void PrintTaskTree(Task* t, bool8 v);

// returns TRUE if failed, FALSE on success
// Intended to be used like if (Exec(...)) { handle_error }
bool8 Exec(Task* t, u32 max_processes);

void BegCompile(TaskGraph* g, bool8 enable_cmds, SString compile_loc);
void EndCompile(TaskGraph* g, Task* t);

Task* Compile(TaskGraph* g, const char* filename, u32 cmd_size, const char** cmd, u32 num_arg, ...);

#ifdef SB_IMPL
#include "../src/core/core.c"

#include <stdarg.h>
#include <unistd.h>

void _AutoRebuild(const char* filename, int argc, char** argv, u32 num_flags,
                  ...) {

    file f1 = filestat((SString){strlen(filename), (i8*)filename});
    file f2 = filestat((SString){strlen(argv[0]), (i8*)argv[0]});
    bool8 rebuild = timecmp(f1.stats.modified, >=, f2.stats.modified);

    if (!rebuild)
        return;

    TaskGraph g = CreateTaskGraph();

    va_list args;
    va_start(args, num_flags);

    Task* t = AddTask(&g, "recompile");
    AddCmd(t, "cc", filename);
    for (u32 i = 0; i < num_flags; i++) {
        const char* arg = va_arg(args, const char*);
        AddCmd(t, arg);
    }
    AddCmd(t, "-o", argv[0]);

    va_end(args);

    if (Exec(t, 1)) {
        debugerr("Failed to recompile");
        panic();
    }

    printlog("Rebuild\n");
    fileflush(&logfile);
    execvp(argv[0], argv);
    debugerr("Failed to Exec Rebuilt Build Script");
    panic();

    FreeTaskGraph(&g);
}

TaskGraph CreateTaskGraph() {
    return (TaskGraph){
        .tasks = InitExar(GlobalAllocator, Task),
        .str_data.a = GlobalAllocator,

        .running.tasks.a = GlobalAllocator,
        .running.processes.a = GlobalAllocator,
    };
}

void ClearTaskGraph(TaskGraph* g) {
    for (u32 i = 0; i < g->tasks.size; i++) {
        Task* e = ExarGet(&g->tasks, i);
        dynFree(e->cmds);
        dynFree(e->pre);
        dynFree(e->post);
    }
    ExarClear(&g->tasks);
    g->str_data.size = 0;
    g->running.tasks.size = 0;
    g->running.processes.size = 0;
}

void FreeTaskGraph(TaskGraph* g) {

    for (u32 i = 0; i < g->tasks.size; i++) {
        Task* e = ExarGet(&g->tasks, i);
        dynFree(e->cmds);
        dynFree(e->pre);
        dynFree(e->post);
    }

    ExarFree(&g->tasks);
    dynFree(g->str_data);
    dynFree(g->running.tasks);
    dynFree(g->running.processes);
}

static u32 AddStr(TaskGraph* t, SString str) {

    dynReserve(t->str_data, t->str_data.size + str.len + 1);
    u32 idx = t->str_data.size;
    memcpy(&t->str_data.data[t->str_data.size], str.data, str.len);
    t->str_data.size += str.len;
    t->str_data.data[t->str_data.size++] = 0;

    return idx;
}

void AddDep(Task* t, Task* d) {
    if (!d) return;
    if (d->cmds.size || d->func) {
        dynPush(t->pre, d);
        return;
    }
    //d must have no effect since d->func == NULL
    //and d->cmds.size == 0

    //close transitives from dummy tasks
    for (u32 i = 0; i < d->pre.size; i++) {
        dynPush(t->pre, d->pre.data[i]);
    }
}

Task* _AddTask(TaskGraph* g, const char* name, u32 num_dep, ...) {
    Task v = {
        .g = g,
        .name = (SString){strlen(name), (i8*)name},
        .cmds.a = GlobalAllocator,
        .post.a = GlobalAllocator,
        .pre.a = GlobalAllocator,
    };
    Task* t = ExarInsert(&g->tasks, &v);

    va_list args;
    va_start(args, num_dep);

    for (u32 i = 0; i < num_dep; i++) {
        Task* d = va_arg(args, Task*);
        AddDep(t, d);
    }

    va_end(args);
    return t;
}

void _AddCmd(Task* t, u32 num_args, ...) {
    if (!t)
        return;

    TaskGraph* g = t->g;

    va_list args;
    va_start(args, num_args);
    for (u32 i = 0; i < num_args; i++) {
        const char* arg = va_arg(args, const char*);
        u32 id = AddStr(g, (SString){strlen(arg), (i8*)arg});
        dynPush(t->cmds, id);
    }
    va_end(args);
}

static void PrintTask(Task* t, CBuf* b, bool8 last, bool8 v) {

    TaskGraph* g = t->g;
    printlog("%s", (SString){b->size, b->data});
    if (last) {
        printlog("\\--");
        dynExt((*b), "   ", 3);
    } else {
        printlog("+--");
        dynExt((*b), "|  ", 3);
    }

    if (v) {
        printlog("%s-", t->name);
        for (u32 i = 0; i < t->cmds.size; i++) {
            printlog(" %n", &g->str_data.data[t->cmds.data[i]]);
        }
        printlog("\n");
    } else {
        printlog("%s\n", t->name);
    }

    for (u32 i = 0; i < t->pre.size; i++) {
        PrintTask(t->pre.data[i], b, i == t->pre.size - 1, v);
    }
    b->size -= 3;
}

// Print tree structure for debugging
void PrintTaskTree(Task* t, bool8 v) {
    if (!t) {
        printlog("Null Task: (May have been Pruned by Condition)\n");
        return;
    }

    CBuf c = {GlobalAllocator};
    PrintTask(t, &c, TRUE, v);
    dynFree(c);
}

#include <stdio.h>

DefDynQueueDecl(TaskQueue, Task*);
DefDynQueueImpl(TaskQueue, Task*);

static void RunTask(Task* t, TaskQueue* q) {
    if (!t) {
        debugerr("Tried to Run NULL Task\n");
        return; // skip
    }

    TaskGraph* g = t->g;

    if (t->func) {
        //run func on main process
        //avoids ipc
        t->visited = FALSE;
        t->func(t);

        for (u32 i = 0; i < t->post.size; i++) {
            Task* n = t->post.data[i];
            n->blockers--;

            if (n->blockers == 0 && (n->func || n->cmds.size)) {
                TaskQueuePush(q, n);
            }
        }
        return;
    }

    pid_t id = fork();

    if (id) {
        // parent
        // printlog("PID: %d\n", id);
        t->visited = FALSE;
        dynPush(g->running.processes, id);
        dynPush(g->running.tasks, t);
        return;
    }
    // child
    ScratchArena a = ScratchArenaGet(NULL);

    // alloc enough for all args + NULL ptr
    char** args = ArenaAllocZero(a.arena, sizeof(char*) * (t->cmds.size + 1));

    //should be impossible to trigger in practice
    //I filter at the Add Task stage
    //and at the scheduling stage, basically if a task is a dummy
    //task it should never get here
    if (!t->cmds.size && !t->func) { 
        fprintf(stderr, "Error, invalid task: %s\n", t->name.data);
        exit(1);
    }

    for (u32 i = 0; i < t->cmds.size; i++) {
        args[i] = &g->str_data.data[t->cmds.data[i]];
    }
    
    execvp(args[0], args);
    panic();
}


#include <sys/wait.h>

static void GatherTasks(ArenaAllocator a, Task* t, TaskQueue* queue) {

    // mark visited
    if (!t->visited) {
        t->visited = TRUE;
        t->blockers = t->pre.size;

        t->post.size = 0;

        if (t->pre.size == 0) {
            TaskQueuePush(queue, t);
        }
    }

    // populate dependents
    for (u32 i = 0; i < t->pre.size; i++) {
        Task* dep = t->pre.data[i];
        GatherTasks(
            a, dep,
            queue); // important Gather Tasks initializes and clears data
                    // So it must run before we start writing to dep's post
        dynPush(dep->post, t);
    }
}

bool8 Exec(Task* t, u32 max_processes) {
    assert(max_processes > 0);
    TaskGraph* g = t->g;
    ScratchArena sc = ScratchArenaGet(NULL);

    TaskQueue queue = TaskQueueCreate(ArenaGetAllocator(sc.arena));
    dynReserve(g->running.tasks, max_processes);
    dynReserve(g->running.processes, max_processes);

    GatherTasks(sc.arena, t, &queue);

    bool8 failed = FALSE;

    // while tasks are in process or queued, continue
    while (!TaskQueueEmpty(&queue) || g->running.processes.size) {
        if (g->running.processes.size >= max_processes ||
            TaskQueueEmpty(&queue)) {
            // wait
            int res;
            pid_t id = wait(&res);

            u32 match = -1;
            for (u32 i = 0; i < g->running.processes.size; i++) {
                if (id == g->running.processes.data[i]) {
                    match = i;
                    break;
                }
            }

            if (!WIFEXITED(res) || WEXITSTATUS(res) != 0 || match == -1) {
                printerr("\e[38;2;255;0;0m[ERR]: Task %s Failed\e[0m\n",
                         g->running.tasks.data[match]->name);
                printerr(
                    "\e[38;2;255;255;0m[WARN]: Cancelling Task Exec: %s\e[0m\n",
                    t->name);
                dynDel(g->running.processes, match);
                dynDel(g->running.tasks, match);
                failed = TRUE;
                break;
            }

            Task* fin = g->running.tasks.data[match];

            for (u32 i = 0; i < fin->post.size; i++) {
                Task* n = fin->post.data[i];
                n->blockers--;

                if (n->blockers == 0 && (n->func || n->cmds.size)) {
                    TaskQueuePush(&queue, n);
                }
            }

            dynDel(g->running.processes, match);
            dynDel(g->running.tasks, match);
        }

        // grab task if possible
        Task* curr = NULL;
        if (TaskQueuePop(&queue, &curr)) {
            // run task
            RunTask(curr, &queue);
        }
    }

    while (g->running.processes.size) {
        // wait
        int res;
        pid_t id = wait(&res);

        u32 match = -1;
        for (u32 i = 0; i < g->running.processes.size; i++) {
            if (id == g->running.processes.data[i]) {
                match = i;
                break;
            }
        }

        if (!WIFEXITED(res) || WEXITSTATUS(res) != 0 || match == -1) {
            printerr("\e[38;2;255;0;0m[ERR]: Task %s Failed\e[0m\n",
                     g->running.tasks.data[match]->name);
            printerr(
                "\e[38;2;255;255;0m[WARN]: Cancelling Task Exec: %s\e[0m\n",
                t->name);
            dynDel(g->running.processes, match);
            dynDel(g->running.tasks, match);
            failed = TRUE;
            break;
        }

        dynDel(g->running.processes, match);
        dynDel(g->running.tasks, match);
    }

    ScratchArenaEnd(sc);
    return failed;
}


//DO NOT TOUCH
//Internal Implementation data, should not be touched
//unless you know what you are doing
static struct {
     bool8 written;
     bool8 enabled;
     SString compile_file;
     file compile_cmds;
     Task* last;
} _c_intern;

task_func(beg_compile, t) {
    _c_intern.compile_cmds = fileopen(_c_intern.compile_file, FILE_WRITE | FILE_CREAT | FILE_TRUNC);
    filewrite(&_c_intern.compile_cmds, sstring("[\n"));
}

task_func(add_entry, t) {
    ScratchArena sc = ScratchArenaGet(NULL);
    TaskGraph* g = t->g;

    char* buffer = ArenaAlloc(sc.arena, PATH_MAX + 1);
    getcwd(buffer, PATH_MAX);
    
    SString compile_entry = sstring(
            "{\n"
            "\t\"directory\": \"%n\",\n"
            "\t\"file\": \"%n\",\n" //artifact
            "\t\"arguments\": [ ");


    if (_c_intern.written) {
        fformat(&_c_intern.compile_cmds, ",\n");
    }
    _c_intern.written = TRUE;

    fformat(&_c_intern.compile_cmds, (char*)compile_entry.data, buffer, &g->str_data.data[t->cmds.data[0]]);

    for (u32 i = 1; i < t->cmds.size; i++) {
        if (i > 1) fformat(&_c_intern.compile_cmds, ",");
        fformat(&_c_intern.compile_cmds, "\"%n\" ", &g->str_data.data[t->cmds.data[i]]);
    }
    fformat(&_c_intern.compile_cmds,
            "]\n"
            "}"
            );


    ScratchArenaEnd(sc);
}

task_func(end_compile, t) {
    _c_intern.written = FALSE;
    filewrite(&_c_intern.compile_cmds, sstring("\n]\n"));
    fileclose(_c_intern.compile_cmds);
}

void BegCompile(TaskGraph* g, bool8 enable_cmds, SString compile_loc) {
    _c_intern.enabled = enable_cmds;

    _c_intern.compile_file = compile_loc;

    Task* start = AddTask(g, "init_compile");
    AddProc(start, beg_compile);
    _c_intern.last = start;
}

/**
 * filename is the name of the final build artifact
*/
Task* Compile(TaskGraph* g, const char* filename, u32 cmd_size, const char** cmd, u32 num_arg, ...) {
    //compilation
    Task* compile = AddTask(g, filename);
    AddCmd(compile, "cc", "-o", filename);

    Task* entry = AddTask(g, filename, _c_intern.last);
    AddProc(entry, add_entry);
    AddCmd(entry, filename);
    _c_intern.last = entry;

    va_list args;
    va_start(args, num_arg);
    for (u32 i = 0; i < num_arg; i++) {
        Task* arg = va_arg(args, Task*);
        AddDep(compile, arg);
    }
    va_end(args);

    for (u32 i = 0; i < cmd_size; i++) {
        AddCmd(compile, cmd[i]);
        AddCmd(entry, cmd[i]);
    }

    return compile;
}

void EndCompile(TaskGraph* g, Task* t) {
    if (!_c_intern.enabled) return; //skips connecting to graph
    Task* end = AddTask(g, "end_compile", _c_intern.last);
    AddProc(end, end_compile);
    AddDep(t, end);
}

#endif
#endif
