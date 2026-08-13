#define SB_IMPL
#include "./include/sbuild/sbuild_st.h"


int main(int argc, char* argv[]) {
    //setdir(sstring("../.."));

    logfile.buffered = FALSE;
    //printlog("src: %n\n", __FILE_NAME__);
    AutoRebuild(argc, argv);

    TaskGraph g = CreateTaskGraph();
    Task* t4 = AddTask(&g, "t4");
    AddCmd(t4, "some", "command");
    Task* t5 = AddTask(&g, "t5");
    AddCmd(t5, "some");

    Task* t2 = AddTask(&g, "t2", t4, t5);
    Task* t3 = AddTask(&g, "t3", 0);

    Task* t1 = AddTask(&g, "t1", t2, t3);
    AddCmd(t1, "echo", "test");

    //PrintTaskTree(t1, TRUE);
    ClearTaskGraph(&g);

    t1 = AddTask(&g, "t1", 0);
    AddCmd(t1, "echo", "Cmd Successful!");
    //RunTask(t1);

    ClearTaskGraph(&g);

    t4 = AddTask(&g, "t4");
    AddCmd(t4, "echo", "t4");

    t5 = AddTask(&g, "t5");
    //AddCmd(t5, "echo", "t5");

    t2 = AddTask(&g, "t2", t4, t5);
    AddCmd(t2, "echo", "t2");

    t3 = AddTask(&g, "t3");
    AddCmd(t3, "echo", "t3");

    t1 = AddTask(&g, "t1", t2, t3, t5);
    AddCmd(t1, "echo", "t1");

    PrintTaskTree(t1, TRUE);
    Exec(t1, 2);

    printlog("Fixed\n");

    AddCmd(t5, "echo", "t5");
    Exec(t1, 2);

    ClearTaskGraph(&g);

    //conditional task adding
    t3 = AddTask(&g, "dep");
    t1 = AddTaskCond(0, &g, "conditional", t3);
    t2 = AddTask(&g, "root", t1);

    PrintTaskTree(t2, TRUE);
    ClearTaskGraph(&g);

    t1 = AddTask(&g, "t1");
    AddCmd(t1, "pwd");
    t2 = AddTask(&g, "t2");
    AddCmd(t2, "pwd");

    t3 = AddDummyTask(&g, t1, t2);

    t4 = AddTask(&g, "t4", t3);
    AddCmd(t4, "pwd");

    PrintTaskTree(t4, FALSE);
    FreeTaskGraph(&g);
    return 0;
}
