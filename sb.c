#include <stdio.h>
#define SB_IMPL
#include "sb.h"


int main(int argc, char* argv[]) {
    sb_BUILD(argc, argv) {
        sb_printf("%d\n", sb_check_arg("test"));
        sb_chdir_exe();
        sb_target_dir("build/");
        sb_mkdir("build");
        sb_EXEC() {
            sb_set_out("app");

            sb_add_source_dir("tests");
            sb_add_flag("g");

            sb_set_incremental();
            sb_export_command();
            sb_set_find_deps();


        }
        sb_fence();
        sb_CMD() {
            sb_cmd_main("./build/app");
        }
    }
    //printf("%s\n", sb_basename("test"));
}
