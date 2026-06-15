#ifndef SB_H
#define SB_H
#include <core/cutils.h>
#include <ds/ds.h>

typedef u32 TaskID;
typedef u32 StrID;

typedef u32 (*taskFunc)(SString* args, u32 arg_num);

typedef struct Task {
    SString name;
    taskFunc f;

    struct {
        u32 start;
        u32 num;
    } args;

    struct {
        u32 start;
        u32 num;
    } reqs;
} Task;

typedef struct TaskContext {
    Task curr;

    //top level task list
    dynArray(Task) tasks; 

    //refs to dependencies
    dynArray(SString) args;    //args
    dynArray(TaskID) reqs;    //No input used But Must run first
} TaskContext;

TaskContext TaskContextCreate(Allocator a);
void TaskContextDestroy(TaskContext* t);

void PrintTaskTree(TaskContext* ctx, TaskID root);

StrID AddStr(TaskContext* ctx, SString string);

void BeginCmd(TaskContext* ctx, SString name);
TaskID PushCmd(TaskContext* ctx);

void AddArg(TaskContext* ctx, SString v);
void AddReq(TaskContext* ctx, TaskID v);

void ExecTree(TaskContext* ctx, TaskID root);

#ifdef SB_IMPL
#include <../src/core/core.c>

TaskContext TaskContextCreate(Allocator a) {
    TaskContext ctx = {
        .tasks = {a},
        .args = {a},
        .reqs = {a},
    };

    return ctx;
}

void TaskContextDestroy(TaskContext* t) {
    dynFree(t->tasks);
    dynFree(t->args);
    dynFree(t->reqs);
}

static dynArray(char) indent;
static void PrintTree(TaskContext* ctx, TaskID v, bool8 last, bool8 first) {
    SString ind = { 
        .data = (i8*)indent.data,
        .len = indent.size
    };
    Task* c = &ctx->tasks.data[v];
    if (first) {
        printlog("%s\n", c->name);
    } else {
        printlog("%s+-%s\n", ind, c->name);
    }


    SString mid = last ? sstring("  ") : sstring("| ");

    dynExt(indent, mid.data, mid.len);
    for (u32 i = 0; i < c->reqs.num; i++) {
        TaskID v = ctx->reqs.data[i + c->reqs.start];
        bool8 end = (i + 1) >= c->reqs.num;
        PrintTree(ctx, v, end, FALSE);
    }
    indent.size -= mid.len;
}

void PrintTaskTree(TaskContext* ctx, TaskID root) {
    //clear indent
    indent.size = 0;

    if (!indent.a.a) {
        indent.a = GlobalAllocator;
    }

    PrintTree(ctx, root, TRUE, TRUE);
}

void BeginCmd(TaskContext* ctx, SString name) {
    ctx->curr = (Task) {
        .name = name,
        .reqs.start = ctx->reqs.size,
        .args.start = ctx->args.size,
    };
}

TaskID PushCmd(TaskContext* ctx) {
    dynPush(ctx->tasks, ctx->curr);
    return ctx->tasks.size - 1;
}

void AddReq(TaskContext* ctx, TaskID v) {
    ctx->curr.reqs.num++;
    dynPush(ctx->reqs, v);
}

void AddArg(TaskContext* ctx, SString s) {
    ctx->curr.args.num++;
    dynPush(ctx->args, s);
}

typedef struct Bucket {
    u32 first;
    u32 num;
} Bucket;

void ScheduleTree(TaskContext* ctx, u32 depth, TaskID curr) {
    Task* c = &ctx->tasks.data[curr];
    for (u32 i = 0; i < c->reqs.num; i++) {
        ScheduleTree(ctx, depth + 1, ctx->reqs.data[i + c->reqs.start]);
    }
}

void ExecTree(TaskContext* ctx, TaskID root) {
    //Schedule
    dynArray(Bucket) schedule = {GlobalAllocator};
    dynArray(TaskID) pass = {GlobalAllocator};

      
             

    //Execute
}

#endif
#endif
