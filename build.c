#include <stdio.h>
#define SB_IMPL
#include "sb.h"
#include <linux/limits.h>

int main(int argc, char *argv[]) {
    sb_BUILD(argc, argv) {
        sb_chdir_exe();
        sb_mkdir("build/");
        sb_mkdir("build/tests");
        sb_target_dir("build/");

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

        // tests
        sb_FOREACHFILE("test/tests/", test) {
            if (sb_cmpext(test, ".c"))
                continue;
            sb_EXEC() {
                sb_add_file(test);
                sb_add_header("include/cutils.h");

                sb_add_include_path("include/");

                sb_add_flag("g");
                sb_add_flag("DCU_IMPL");
                sb_link_library("m");

                char buf[PATH_MAX + 1] = {0};
                char final[PATH_MAX + 1] = {0};
                strncpy(buf, test, PATH_MAX);

                char *name = sb_stripext(sb_basename(buf));
                snprintf(final, PATH_MAX, "tests/%s", name);

                sb_set_out(final);

                sb_export_command();
            }
        }

        sb_EXEC() {
            sb_add_file("test/runner.c");
            sb_add_header("include/cutils.h");

            sb_add_include_path("include/");

            sb_add_flag("g");
            sb_add_flag("DCU_IMPL");
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
        sb_CMD() { sb_cmd_main("build/runner"); }
        sb_build_end();
    }
}
