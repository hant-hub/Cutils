#ifndef SB_H
#define SB_H
#include <core/cutils.h>

#ifdef SB_IMPL
#define DS_IMPL
#endif
#include <ds/ds.h>

typedef struct TaskGraph TaskGraph;

typedef struct Task {
    TaskGraph* g; // back ref
    SString name; // for debug

    bool8 visited;
    u32 blockers;

    dynArray(struct Task*) pre;  // things this task depends on
    dynArray(struct Task*) post; // things which depend on this task
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

#define AddTask(g, name, ...)                                                  \
    _AddTask(g, name, ARRAY_SIZE(((Task*[]){__VA_ARGS__})), ##__VA_ARGS__)

#define AddTaskCond(cond, g, name, ...)                                        \
    (cond ? AddTask(g, name, ##__VA_ARGS__) : NULL)

#define AddCmd(t, ...)                                                         \
    _AddCmd(t, ARRAY_SIZE(((const char*[]){__VA_ARGS__})), ##__VA_ARGS__)

TaskGraph CreateTaskGraph();
void ClearTaskGraph(TaskGraph* g);
void FreeTaskGraph(TaskGraph* g);
Task* _AddTask(TaskGraph* g, const char* name, u32 num_dep, ...);
void _AddCmd(Task* t, u32 num_args, ...);

void PrintTaskTree(Task* t, bool8 v);

// returns TRUE if failed, FALSE on success
bool8 Exec(Task* t, u32 max_processes);

#ifdef SB_IMPL
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
        if (d)
            dynPush(t->pre, d);
        // dynPush(d->post, t);
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

static void RunTask(Task* t) {
    if (!t) {
        debugerr("Tried to Run NULL Task\n");
        return; // skip
    }

    TaskGraph* g = t->g;

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

    if (!t->cmds.size) {
        fprintf(stderr, "Error, invalid task: %s\n", t->name.data);
    }

    for (u32 i = 0; i < t->cmds.size; i++) {
        args[i] = &g->str_data.data[t->cmds.data[i]];
    }

    execvp(args[0], args);

    panic();
}

DefDynQueueDecl(TaskQueue, Task*);
DefDynQueueImpl(TaskQueue, Task*);

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

                if (n->blockers == 0) {
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
            RunTask(curr);
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

#endif
#endif
