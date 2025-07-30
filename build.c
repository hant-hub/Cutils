#include <linux/limits.h>
#define SB_IMPL
#include "sb.h"

int main(int argc, char *argv[]) {
    sb_BUILD(argc, argv) { 
        sb_chdir_exe();
        sb_mkdir("build/");
        sb_target_dir("build/");
        //tests
        sb_FOREACHFILE("test/", test) {
            if (sb_cmpext(test, ".c") == 0) continue;
            sb_EXEC() {
                sb_add_file("src/cutils.c");
                sb_add_file(test);

                sb_add_include_path("include/");

                sb_add_flag("g");
                sb_link_library("m");

                char buf[PATH_MAX + 1] = {0};
                strncpy(buf, test, PATH_MAX);

                sb_set_out(sb_stripext(sb_basename(buf)));

                sb_export_command();
            }
        }
        sb_fence();
        sb_FOREACHFILE("build/", test) {
            if (sb_cmpext(test, ".c") == 0) continue;
            sb_CMD() {
                sb_cmd_main(test);
            }
        }
    }
}
