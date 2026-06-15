#define SB_IMPL
#include <sbuild/sbuild.h>


int main() {
    TaskContext ctx = TaskContextCreate(GlobalAllocator);
    SString e = sstring("echo");
    SString t = sstring("test");

    BeginCmd(&ctx, sstring("e2"));
    AddArg(&ctx, t);
    TaskID e2 = PushCmd(&ctx);

    BeginCmd(&ctx, sstring("e3"));
    AddReq(&ctx, e2);
    AddArg(&ctx, t);
    TaskID e3 = PushCmd(&ctx);

    BeginCmd(&ctx, sstring("e4"));
    AddArg(&ctx, t);
    TaskID e4 = PushCmd(&ctx);

    BeginCmd(&ctx, sstring("root"));
    AddReq(&ctx, e3);
    AddReq(&ctx, e4);
    AddArg(&ctx, e);
    TaskID root = PushCmd(&ctx);

    PrintTaskTree(&ctx, root);
    TaskContextDestroy(&ctx);
    return 0;
}
