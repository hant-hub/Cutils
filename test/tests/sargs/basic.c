#include <sargs/sa_pre.h>

#define PROG_DESC "This is a test description"

#define REQS                 \
    ARG(a1, int, "a1 usage") \
    ARG(a2, int, "a2 usage")

#define OPTS                 \
    OPT(a3, int, "a1 error")

#define FLAGS                \
    FLAG(a4, arg4, int, "a4 error") \
    FLAG(a5, arg5, int, "a5 error") \
    FLAG(a6, arg6, bool8, "a6 error") \
    FLAG(a7, arg7, bool8, "a7 error") \
    FLAG(a8, arg8, char*, "a8 error")


#include <sargs/sa.h>
#include <core/cutils.h>

int main() {
    logfile->buffered = FALSE;
    {
        char* argv[] = {
            "",
            "1",
            "2",
            "test",
            "-a4=2",
            "-a5",
            "3",
            "-a6",
            "--arg7",
        };

        SA v = ParseArgs(ARRAY_SIZE(argv), argv);
        assert(!v.err);
        assert(v.args.a1 == 1);
        assert(v.args.a2 == 2);
        assert(v.opts.a3.p == FALSE);

        assert(v.flags.a4.p == TRUE);
        assert(v.flags.a4.v == 2);

        assert(v.flags.a5.p == TRUE);
        assert(v.flags.a5.v == 3);

        assert(v.flags.a6.p == TRUE);
        assert(v.flags.a6.v == TRUE);

        assert(v.flags.a7.p == TRUE);
        assert(v.flags.a7.v == TRUE);
    }

    {
        char* argv[] = {
            "",
            "1",
            "2",
            "test",
            "-a4=2",
            "-a5",
            "-a6",
            "--arg7",
        };

        SA v = ParseArgs(ARRAY_SIZE(argv), argv);
        assert(v.err);
        printlog("%n\n", v.err);
    }

    {
        char* argv[] = {
            "",
            "1",
            "2",
            "test",
            "-a4=2",
            "-a6",
            "--arg7",
            "--arg8=test",
        };

        SA v = ParseArgs(ARRAY_SIZE(argv), argv);
        assert(v.flags.a8.p);
        printlog("%n\n", v.flags.a8.v);
    }

    {
        char* argv[] = {
            "",
            "1",
            "2",
            "test",
            "-a4=2",
            "-a6",
            "--arg7",
            "--arg8",
            "test",
        };

        SA v = ParseArgs(ARRAY_SIZE(argv), argv);
        assert(v.flags.a8.p);
        printlog("%n\n", v.flags.a8.v);
    }
    {
        char* argv[] = {
            "",
            "1",
            "2",
            "test",
            "-a4=2",
            "-a6",
            "--arg7",
            "--arg8",
            "-a5=2",
        };

        SA v = ParseArgs(ARRAY_SIZE(argv), argv);
        printlog("%n\n", v.err);
        assert(v.err);
        assert(v.flags.a8.p);
    }

    {
        char* argv[] = {
            "",
            "--help",
        };

        SA v = ParseArgs(ARRAY_SIZE(argv), argv);
        printlog("%n\n", v.err);
        assert(v.err);
        assert(strcmp(v.err, "User Displayed Help Info") == 0);
    }

    {
        char* argv[] = {
            "",
            "-h",
        };

        SA v = ParseArgs(ARRAY_SIZE(argv), argv);
        printlog("%n\n", v.err);
        assert(v.err);
        assert(strcmp(v.err, "User Displayed Help Info") == 0);
    }
}
