#ifndef SA_H
#define SA_H
#include <core/cutils.h>
#include <string.h>

#include <../src/core/core.c>

#ifndef PROG_NAME
#define PROG_NAME "prog_name"
#endif

#ifndef REQS
#define REQS
#endif

#ifndef OPTS
#define OPTS
#endif

#ifndef FLAGS
#define FLAGS
#endif

#undef ARG
#undef OPT
#undef FLAG

typedef struct SA {
    char* err;
    struct {
    #define ARG(name, type, usage) type name;
    REQS 
    #undef ARG
    } args;

    struct {
    #define OPT(sname, type, usage) struct { type v; bool8 p; } sname;
    OPTS 
    #undef OPT
    } opts;

    struct {
    #define FLAG(sname, lname, type, usage) struct { type v; bool8 p; } sname;
    FLAGS
    #undef FLAG
    } flags;
} SA;

static int StrtoStr(SString str, char** out) {
    *out = (char*)str.data;
    return str.len;
}

static int StrBool(SString str, bool8* out) {
    *out = TRUE;
    return 1;
}

#define ARG(name, type, usage) "\t" _QUOTE(name) ": " _QUOTE(type) "\t\t" usage "\n"
#define OPT(name, type, usage) "\t" _QUOTE(name) ": " _QUOTE(type) "\t\t" usage "\n"
#define FLAG(sname, lname, type, usage) "\t" "-"_QUOTE(sname) ", " "--" _QUOTE(lname)": " _QUOTE(type) "\t\t" usage "\n"

static const SString helpmsg = sstring(
    PROG_NAME  "  [ARGS] [OPTIONAL ARGS]\n"
    "ARGS:\n"
    REQS
    "OPTIONAL ARGS:\n"
    OPTS
    "FLAGS:\n"
    FLAGS
    "\n"
);
#undef ARG
#undef OPT
#undef FLAG

#define parseval(out, str) _Generic(out, \
    int: StrtoI32, \
    char*: StrtoStr, \
    bool8: StrBool \
)(str, &out)

SA ParseArgs(int argc, char* argv[]) {
    SA args = {};
    u32 curr = 1; //skip prog name

    //check help
    SString str_h = sstring("-h"); 
    SString str_help = sstring("--help"); 
    for (u32 i = 1; i < argc; i++) { 
        char* c = argv[i]; 
        if (strncmp(c, (char*)str_h.data, str_h.len) == 0) { 
            printlog("%s\n", helpmsg);
            args.err = "User Displayed Help Info"; 
            return args; 
        } 
        if (strncmp(c, (char*)str_help.data, str_help.len) == 0) {
            printlog("%s\n", helpmsg);
            args.err = "User Displayed Help Info"; 
            return args;
        }
    }

    //parse required args

    #define ARG(name, type, usage) \
        do { \
            if (curr >= argc) { \
                args.err = usage; \
                return args; \
            } \
            SString str = { \
                .data = (i8*)argv[curr], \
                .len = strlen(argv[curr]), \
            }; \
            parseval(args.args.name, str); \
            curr++; \
        } while(0);
    REQS
    #undef ARG

    //parse Optional args
    #define OPT(name, type, usage) \
        do { \
            if (curr >= argc) { \
                break; \
            } \
            SString str = { \
                .data = (i8*)argv[curr], \
                .len = strlen(argv[curr]), \
            }; \
            int d = parseval(args.opts.name.v, str); \
            if (d == 0) break; \
            args.opts.name.p = TRUE; \
            curr++; \
        } while(0);
    OPTS
    #undef OPT

    //parse flags
    #define FLAG(sname, lname, type, usage) \
        SString str_##sname = sstring("-" #sname); \
        SString str_##lname = sstring("--" #lname); \
        for (u32 i = 0; i < argc; i++) { \
            char* c = argv[i]; \
            if (strncmp(c, (char*)str_##sname.data, str_##sname.len) == 0) { \
                args.flags.sname.p = TRUE; \
                c += str_##sname.len; \
                if (c[0] == '=') { \
                    c++; \
                } else if (c[0] == 0) { \
                    i++; \
                    if (i >= argc) { \
                        i--; \
                    } else { \
                        if (argv[i][0] != '-') c = argv[i]; \
                    }\
                } \
                SString str = { \
                    .data = (i8*)c, \
                    .len = strlen(c), \
                }; \
                int d = parseval(args.flags.sname.v, str); \
                if (d == 0) { \
                    args.err = usage; \
                    return args; \
                } \
                break; \
            } \
            if (strncmp(c, (char*)str_##lname.data, str_##lname.len) == 0) { \
                args.flags.sname.p = TRUE; \
                c += str_##lname.len; \
                if (c[0] == '=') { \
                    c++; \
                } else if (c[0] == 0) { \
                    i++; \
                    if (i >= argc) { \
                        i--; \
                    } else { \
                        if (argv[i][0] != '-') c = argv[i]; \
                    }\
                } \
                SString str = { \
                    .data = (i8*)c, \
                    .len = strlen(c), \
                }; \
                int d = parseval(args.flags.sname.v, str); \
                if (d == 0) { \
                    args.err = usage; \
                    return args; \
                } \
                break; \
            } \
        }
    FLAGS
    #undef FLAG


    return args; 
}

#undef parseval
#undef REQ
#undef OPTS
#undef FLAGS

#endif
