#define SB_IMPL
#include "sb.h"

#define OPTS \
    X(t, string)

#include "sa.h"

int main(int argc, char *argv[]) {
    SA options = Parse(argc, argv);
    if (options.err) exit(-1);
    SAvals opts = options.vals;

    printf("Opts: T = %s\n", opts.t.val);

    sb_BUILD(argc, argv) {
        sb_chdir_exe();
        sb_mkdir("build/");
        sb_mkdir("build/tests");
        sb_target_dir("build/");

        if (sb_check_arg("format")) {
            // formatting
            sb_FOREACHFILE("./", test) {
                if (sb_cmpext(test, ".c") && sb_cmpext(test, ".h"))
                    continue;

                sb_CMD() {
                    sb_cmd_main("clang-format");
                    sb_cmd_opt("i");
                    sb_cmd_arg(test);
                }
            }
            sb_FOREACHFILE("test/", test) {
                if (sb_cmpext(test, ".c") && sb_cmpext(test, ".h"))
                    continue;

                sb_CMD() {
                    sb_cmd_main("clang-format");
                    sb_cmd_opt("i");
                    sb_cmd_arg(test);
                }
            }
            sb_FOREACHFILE("include/", test) {
                if (sb_cmpext(test, ".c") && sb_cmpext(test, ".h"))
                    continue;

                sb_CMD() {
                    sb_cmd_main("clang-format");
                    sb_cmd_opt("i");
                    sb_cmd_arg(test);
                }
            }
            sb_FOREACHFILE("./", test) {
                if (sb_cmpext(test, ".c") && sb_cmpext(test, ".h"))
                    continue;

                sb_CMD() {
                    sb_cmd_main("clang-format");
                    sb_cmd_opt("i");
                    sb_cmd_arg(test);
                }
            }
            sb_fence();
        }

        //build Static Impl
        sb_EXEC() {
            sb_add_flag("c");
            sb_add_include_path("include/");
            sb_add_file("src/cutils.c");

            sb_add_flag("g");
            sb_link_library("m");

            sb_set_out("cutils.o");
        }
        sb_fence();
        sb_CMD() {
            sb_cmd_main("ar");
            sb_cmd_arg("rcs");
            sb_cmd_arg("build/cutils.a");
            sb_cmd_arg("build/cutils.o");
        }
        sb_fence();

        sb_CMD() {
            sb_cmd_main("cp");
            sb_cmd_opt("r");
            sb_cmd_arg("resources");
            sb_cmd_arg("build/resources");
        }
        sb_fence();
        // tests
        char* grouplist[] = {
            "tests/core",
            "tests/simpleds",
            "tests/strbase",
            "tests/ga",
            "tests/cgen",
        };
        for (int gid = 0; gid < (sizeof(grouplist)/sizeof(grouplist[0])); gid++) {
            char buf[PATH_MAX + 1] = {0};
            snprintf(buf, PATH_MAX, "build/%s", grouplist[gid]);
            sb_mkdir(buf);
            snprintf(buf, PATH_MAX, "test/%s/", grouplist[gid]);

            sb_FOREACHFILE(buf, test) {
                if (sb_cmpext(test, ".c"))
                    continue;
                sb_EXEC() {
                    sb_add_file(test);
                    sb_add_file("build/cutils.a");
                    sb_add_header("include/cutils.h");

                    sb_add_include_path("include/");

                    sb_add_flag("g");
                    sb_add_flag("DCU_IMPL");
                    sb_link_library("m");

                    char buf[PATH_MAX + 1] = {0};
                    char final[PATH_MAX + 1] = {0};
                    strncpy(buf, test, PATH_MAX);

                    char *name = sb_stripext(sb_basename(buf));
                    snprintf(final, PATH_MAX, "%s/%s", grouplist[gid], name);
                    printf("test: %d %s\n", gid, final);

                    sb_set_out(final);

                    sb_export_command();
                }
            }
        }

        sb_EXEC() {
            sb_add_file("test/runner.c");
            sb_add_header("include/cutils.h");

            sb_add_include_path("include/");
            sb_add_file("build/cutils.a");

            sb_add_flag("g");
            sb_link_library("m");

            sb_set_out("runner");

            sb_export_command();
        }
    }

    if (!sb_check_arg("no-test")) {
        sb_build_start(argc, argv);
        sb_target_dir("build/");
        sb_CMD() { sb_cmd_main("clear"); }
        sb_fence();
        sb_CMD() { 
            sb_cmd_main("build/runner"); 

            if (sb_check_arg("v") || opts.t.set) {
                sb_cmd_arg("v");
            }

            if (opts.t.set) {
                sb_cmd_arg(opts.t.val);
            }
        }
        sb_build_end();
    }
}
