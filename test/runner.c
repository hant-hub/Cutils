#define CU_IMPL
#include <cutils.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

char *sb_stripext(char *f) {
    char *cursor = f;
    while (cursor[0]) cursor++;
    while (cursor[0] != '/') cursor--;
    cursor[0] = 0;
    return f;
}

void runtest(const char *name, int verbose) {
    if (fork()) {
        return;
    }

    if (!verbose) {
        int fd = open("/dev/null", O_RDWR);
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
    }

    execlp("valgrind", "valgrind", "--leak-check=full", "--error-exitcode=69",
           "--", name, NULL);

    printf("failed to find file\n");
    exit(-1);
}

u32 printheader(char *name, int verbose) {
    if (verbose) {
        printf("================\n");
        printf("Running Test: %s\n", name);
        printf("================\n");
        return 0;
    }

    return printf("- %s...", name);
}

void printstatus(u32 shift, int verbose) {
    int status = 0;
    if (verbose) {
        while (waitpid(0, &status, 0) > 0);
        return;
    }

    printf("%*s", 20 - shift, " ");

    while (waitpid(0, &status, 0) > 0) {
        if (!WIFEXITED(status) || WIFSIGNALED(status)) {
            printf("\033[38;2;255;0;0m Failed\033[0m\n");
            return;
        }

        if (WEXITSTATUS(status) == 69) {
            printf("\033[38;2;255;255;0m Leak\033[0m\n");
            return;
        }

        if (WEXITSTATUS(status)) {
            printf("\033[38;2;255;0;0m Failed : %d\033[0m\n",
                   WEXITSTATUS(status));
            return;
        }
    }

    printf("\033[38;2;0;255;0m Passed\033[0m\n");
}

int main(int argc, char *argv[]) {

    char tmp[PATH_MAX] = {0};
    if (readlink("/proc/self/exe", tmp, PATH_MAX) < 0) {
        exit(-1);
    }
    chdir(sb_stripext(tmp));

    DIR *d = opendir("tests/");

    if (!d) {
        debugerr("Failed to open tests");
        exit(-1);
    }

    struct dirent *dir = NULL;

    int verbose = 0;
    if (argc > 1) {
        verbose = 1;
    }

    printf("   +=================+\n"
           "   |  Utility Tests  |\n"
           "   +=================+\n");

    do {
        dir = readdir(d);
        if (dir && dir->d_type != DT_DIR) {
            char buf[PATH_MAX + 1] = {0};

            u32 shift = printheader(dir->d_name, verbose);

            snprintf(buf, PATH_MAX, "./tests/%s", dir->d_name);
            runtest(buf, verbose);
            printstatus(shift, verbose);
        }
    } while (dir);
}
