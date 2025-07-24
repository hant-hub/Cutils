#ifndef SB_BUILD_H
#define SB_BUILD_H

#include <signal.h>
#include <stdint.h>

/*
 * The main idea is that sb_cmd
 * will differentiate each
 * command. On Linux each command
 * will be seperated by Null terminators
 * to allow for easy construction of the
 * arguement vector
 *
 * On Windows each argument is seperated by 
 * spaces, and each command by a null
 * terminator allowing for easy usage here.
 *
 */

#ifdef _MSC_VER
typedef void* sb_file;
static sb_file error = NULL;
#else
typedef int sb_file;
static sb_file error = -1;
#endif

typedef enum sb_platform_type {
    sb_WIN32,
    sb_UNIX,
    sb_OSX,
} sb_platform_type;

typedef enum sb_file_mode {
    sbf_READ,
    sbf_WRITE,
    sbf_READWRITE,
} sb_file_mode;

typedef enum sb_file_flags {
    sbf_APPEND,
    sbf_CREATE,
    sbf_TRUNC,
} sb_file_flags;

//platform utilities
sb_file sb_open(char* file, sb_file_mode mode, sb_file_flags flags);
void sb_close(sb_file f);

void sb_delete_file(char* name);
void sb_delete_dir(char* name);
void sb_mkdir(char* name);
void sb_chdir(char* name);
void sb_chdir_exe();

void sb_fprintf(sb_file f, char* format, ...);
void sb_printf(char* format, ...);
void sb_snprintf(void* dst, uint32_t max, char* format, ...);

void* sb_mapfile(sb_file f, uint32_t* size);
void sb_readfile(void* dst, sb_file f, uint32_t num_bytes);

void sb_exit(int status);

char* sb_get_cwd();

//Memory allocation
void* sb_alloc(uint64_t size);
void* sb_realloc(void* p, uint64_t size);
void sb_free(void* p);

//String Utilities

typedef struct sb_sized_string {
    char* string;
    uint32_t size;
} sb_sized_string;

//Null terminated
uint32_t sb_strlen(const char* s);
uint32_t sb_strcmp(const char* s1, const char* s2);
char* sb_basename(char* f);

//Sized
void sb_strcpy(char* dst, const sb_sized_string s);

//Conditional functions
sb_platform_type sb_platform();
int sb_check_arg(const char* arg);


/*
 * Non specific Build Functions
 */

int sb_build_start(int argc, char* argv[]);
void sb_build_end();

int sb_cmd_start();
void sb_cmd_end();

void _sb_cmd_main(sb_sized_string cmd);
void _sb_cmd_opt(sb_sized_string opt);
uint32_t _sb_cmd_arg(sb_sized_string arg);

void sb_autobuild(int argc, char* argv[], char* src);
int sb_cmptime(const char* f1, const char* f2);

/* forces all previous commands to finish before continuing*/
void sb_fence();


/*
 * Compiler specific functions 
 */

int sb_start_exec();
void sb_stop_exec();

void _sb_add_file(sb_sized_string f);
void _sb_add_source_dir(sb_sized_string f);
void _sb_add_header(sb_sized_string f);
void _sb_set_out(sb_sized_string f);
void _sb_target_dir(sb_sized_string f);
void _sb_add_flag(sb_sized_string f);

void _sb_add_include_path(sb_sized_string f);
void _sb_add_library_path(sb_sized_string f);
void _sb_link_library(sb_sized_string f);

void sb_set_optmize(uint32_t level);
void sb_export_command();
void sb_set_incremental();

//This is optional, you can manually ad dependencies via
//add header, but this will utilize the compiler emitted
//header dependencies.
void sb_set_find_deps();

/* 
 * will run through build logic but won't submit to 
 * execution list. Useful for creating compile
 * commands without actually compiling
 */
void sb_dry_run();



/*
 * Spooky Macros
 */

#define sb_BUILD(argc, argv) \
    for (int i = (sb_build_start(argc, argv), sb_autobuild(argc, argv, __FILE__), 0); i == 0; (sb_build_end(), i++))

#define sb_CMD() \
    for (int i = sb_cmd_start(); i == 0; (sb_cmd_end(), i++))

#define sb_EXEC() \
    for (int i = sb_start_exec(); i == 0; (sb_stop_exec(), i++))

#define sb_add_include_path(x) \
    _sb_add_include_path((sb_sized_string){ \
            .string = x,\
            .size = sb_strlen(x),\
            })

#define sb_add_library_path(x) \
    _sb_add_library_path((sb_sized_string){ \
            .string = x,\
            .size = sb_strlen(x),\
            })

#define sb_link_library(x) \
    _sb_link_library((sb_sized_string){ \
            .string = x,\
            .size = sb_strlen(x),\
            })

#define sb_add_file(x) \
    _sb_add_file((sb_sized_string){ \
            .string = x,\
            .size = sb_strlen(x),\
            })

#define sb_add_source_dir(x) \
    _sb_add_source_dir((sb_sized_string){ \
            .string = x,\
            .size = sb_strlen(x),\
            })


#define sb_add_header(x) \
    _sb_add_header((sb_sized_string){ \
            .string = x,\
            .size = sb_strlen(x),\
            })

#define sb_set_out(x) \
    _sb_set_out((sb_sized_string){ \
            .string = x,\
            .size = sb_strlen(x),\
            })

#define sb_add_flag(x) \
    _sb_add_flag((sb_sized_string){ \
            .string = x,\
            .size = sb_strlen(x),\
            })

#define sb_target_dir(x) \
    _sb_target_dir((sb_sized_string){ \
            .string = x,\
            .size = sb_strlen(x),\
            })

#define sb_cmd_main(x) \
    _sb_cmd_main((sb_sized_string){ \
            .string = x,\
            .size = sb_strlen(x),\
            })

#define sb_cmd_opt(x) \
    _sb_cmd_opt((sb_sized_string){ \
            .string = x,\
            .size = sb_strlen(x),\
            })

#define sb_cmd_arg(x) \
    _sb_cmd_arg((sb_sized_string){ \
            .string = x,\
            .size = sb_strlen(x),\
            })

#ifdef SB_IMPL
/*
 * Just for now, The plan is to use internal macro switches.
 * This is to see if it makes a more readable cross platform
 * file. That way all the code for a specific function
 * is in a single place
 *
 */
#ifdef _MSC_VER
//Windows
#else
//Everything else (currently just linux)
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include <assert.h>
#include <linux/limits.h>
#include <sys/types.h>

#include <sys/mman.h>

#endif

#include <stdarg.h>

typedef struct sb_cmd {
    int32_t start;
    int32_t length;
} sb_cmd;

typedef struct sb_cmd_list {
    //Cmdline Args
    uint32_t cmdsize;
    char** cargs;

    //text buffer
    uint32_t size;
    uint32_t cap;
    char* cmd;

    //indicies
    uint32_t isize;
    uint32_t icap;
    sb_cmd* indicies;
} sb_cmd_list;

sb_file sb_open(char* file, sb_file_mode mode, sb_file_flags flags) {
    int o_mode = 0;
    switch (mode) {
        case sbf_READ: 
            o_mode = O_RDONLY;
            break;
        case sbf_WRITE: 
            o_mode = O_WRONLY;
            break;
        case sbf_READWRITE: 
            o_mode = O_RDWR;
            break;
    }

    if (flags & sbf_CREATE) {
        o_mode |= O_CREAT;
    }

    if (flags & sbf_TRUNC) {
        o_mode |= O_TRUNC;
    }

    return open(file, o_mode, 0644);
}

char cwd[PATH_MAX] = {0}; 

char* sb_get_cwd() {
    if (!cwd[0]) {
        getcwd(cwd, PATH_MAX);    
    }
    return cwd;
}


void sb_exit(int status) {
   exit(status);
}


void sb_fprintf(sb_file f, char* format, ...) {
    va_list args;
    va_start(args, format);
    vdprintf(f, format, args);
    va_end(args);
}

void sb_printf(char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void sb_snprintf(void* dst, uint32_t max, char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(dst, max, format, args);
    va_end(args);
}

void sb_close(sb_file f) {
    close(f);
}

void sb_delete_file(char* name) {
    unlink(name);
}

void sb_delete_dir(char* name) {
    rmdir(name);
}

void sb_mkdir(char* name) {
    mkdir(name, 0777);
}

void* sb_mapfile(sb_file f, uint32_t* size) {
    struct stat buf;
    if (fstat(f, &buf)) {
        sb_printf("Failed to map file!\n");
        sb_exit(-1);
    }
    *size = buf.st_size;

    return mmap(NULL, buf.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, f, 0);
}

void sb_unmapfile(sb_file f, void* map) {
    struct stat buf;
    if (fstat(f, &buf)) {
        sb_printf("Failed to map file!\n");
        sb_exit(-1);
    }

    munmap(map, buf.st_size);
}

void sb_readfile(void* dst, sb_file f, uint32_t num_bytes) {
    assert(0);
}

void sb_chdir(char* name) {
    chdir(name);
}

void sb_chdir_exe() {
    char tmp[PATH_MAX] = {0};
    if (readlink("/proc/self/exe", tmp, PATH_MAX) < 0) {
        sb_printf("Error: Failed to find Build Binary\n");
        sb_exit(-1);
    }
    sb_chdir(tmp);
}

void sb_strcpy(char* dst, const sb_sized_string s) {
    memcpy(dst, s.string, s.size);
}

uint32_t sb_strlen(const char* s) {
    return strlen(s);
}

uint32_t sb_strcmp(const char* s1, const char* s2) {
    return strcmp(s1, s2);
}

char* sb_basename(char* f) {
    char* stripped = f;
    while (stripped[0]) stripped++;
    while (stripped >= f && stripped[0] != '/') stripped--;
    stripped++;
    return stripped;
}

void* sb_alloc(uint64_t size) {
    return malloc(size);
}
void* sb_realloc(void* p, uint64_t size) {
    return realloc(p, size);
}
void sb_free(void* p) {
    return free(p);
}

int sb_cmptime(const char* binary, const char* source) {
    struct stat file1 = {0};
    struct stat file2 = {0};

    int a = stat(binary, &file1);
    int b = stat(source, &file2);

    if (b) {
        sb_printf("Source Not Found\n");
       sb_exit(-1);
    }

    if (a) {
        sb_printf("Binary Not Found\n");
        return 1;
    }

    return file1.st_mtim.tv_sec < file2.st_mtim.tv_sec;
}

static uint32_t sb_cmpext(const char* file, const char* ext) {
    const char* end = file;
    while (end[0]) end++;
    end--;

    while (end > file && end[0] != '.') end--;
    if (end <= file) return -1; //no extension
    end++;

    return sb_strcmp(end, ext);

}

/* ----------------------------------------------------
 * Global Variables
 *
 * VERY IMPORTANT
 *
 */

//may expand later
typedef struct exe_info {
    int export_commands;
    int dry;
    int incremental;
    int auto_deps;

    //unique options
    uint32_t output;

    //options
    uint32_t osize;
    uint32_t ocap;
    uint32_t* options;

    //headers
    uint32_t hsize;
    uint32_t hcap;
    uint32_t* headers;

    //indicies for source files
    uint32_t fsize;
    uint32_t fcap;
    uint32_t* files;

    //raw text
    uint32_t tsize;
    uint32_t tcap;
    char* text;
} exe_info;

sb_cmd_list cmd_list = {0};
sb_cmd curr_cmd = {0};
exe_info curr_exe;

char build_dir[PATH_MAX] = {0};

#ifdef _MSC_VER
int compile_cmds = NULL;

char separator = ' ';
char flag = '/';
#else
sb_file compile_cmds = 0;

char separator = 0;
char flag = '-';
#endif


#ifdef _MSC_VER
//msvc
sb_sized_string compiler = (sb_sized_string) {
    .string = "cl.exe",
        .size = 6
};
#elif defined(__GNUC__)
//gcc
const sb_sized_string compiler = {
    .string = "cc",
    .size = 2
};
#elif defined(__clang__)
//clang
sb_sized_string compiler = (sb_sized_string) {
    .string = "clang",
        .size = 5
};
#endif


#if defined(_WIN32)
const sb_platform_type os = sb_WIN32;  
#elif defined (unix)
const sb_platform_type os = sb_UNIX;  
#elif defined (__APPLE__)
const sb_platform_type os = sb_OSX;  
#endif

sb_platform_type sb_platform() {
    return os;
}

int sb_check_arg(const char* arg) {
    for (uint32_t i = 0; i < cmd_list.cmdsize; i++) {
        if (sb_strcmp(arg, cmd_list.cargs[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

//-----------------------------------------------------

int sb_build_start(int argc, char* argv[]) {
    //reset top pointers,
    //don't mess with pointers
    cmd_list.size = 0;
    cmd_list.isize = 0;

    cmd_list.cmdsize = argc;
    cmd_list.cargs = argv;

    curr_exe = (exe_info){0};


    return 0;
}


void sb_build_end() {
    //execute commands
    //in parrallel

    for (uint32_t i = 0; i < cmd_list.isize; i++) {
        sb_cmd idx = cmd_list.indicies[i];

        if (idx.start < 0) {
            //fence
            int status;
            while (waitpid(0, &status, 0) > 0) {
                if (!WIFEXITED(status)) {
                    sb_printf("Error: %d\n", status);
                   sb_exit(-1);
                }
                if (WEXITSTATUS(status)) {
                    sb_printf("Nonzero Exit: %d\n", status);
                   sb_exit(-1);
                }

            }
            continue;
        }

        pid_t p = fork();
        if (p) continue;
        char* file = &cmd_list.cmd[idx.start];

        //build list
        char** args = sb_alloc(sizeof(char*) * idx.length + 1);

        char* cur = &cmd_list.cmd[idx.start];
        for (uint32_t j = 0; j < idx.length; j++) {
            args[j] = cur;
            sb_printf("%s ", cur);
            //consume
            while (cur[0]) cur++;
            cur++;
        }
        sb_printf("\n");

        args[idx.length] = 0;
        execvp(file, args);
        sb_printf("Failed to Run Command\n");
       sb_exit(-1);
    }

    int status;
    while (waitpid(0, &status, 0) > 0){
        if (!WIFEXITED(status)) {
            sb_printf("Error: %d\n", status);
           sb_exit(-1);
        }
        if (WEXITSTATUS(status)) {
            sb_printf("Nonzero Exit: %d\n", status);
           sb_exit(-1);
        }
    }


    if (compile_cmds) {
        sb_fprintf(compile_cmds, "\n]\n");
        sb_close(compile_cmds);
    }
}

int sb_cmd_start() {
    curr_cmd.start = cmd_list.size;
    curr_cmd.length = 0;
    return 0;
}

void sb_cmd_end() {
    if (cmd_list.isize + 1 > cmd_list.icap) {
        cmd_list.icap = cmd_list.icap ? cmd_list.icap * 2 : 16;
        cmd_list.indicies = sb_realloc(cmd_list.indicies, cmd_list.icap * sizeof(sb_cmd));
    }

    if (curr_exe.fsize && curr_exe.dry) {
        cmd_list.size = curr_cmd.start;
        return;
    }

    cmd_list.indicies[cmd_list.isize++] = curr_cmd;
}

void _sb_cmd_main(sb_sized_string cmd) {
    curr_cmd.length++;

    if (cmd_list.size + cmd.size > cmd_list.cap) {
        cmd_list.cap = cmd_list.cap ? cmd_list.cap * 2 : 256;
        cmd_list.cmd = sb_realloc(cmd_list.cmd, cmd_list.cap);
    }

    sb_strcpy(&cmd_list.cmd[cmd_list.size], cmd);
    cmd_list.size += cmd.size;
    cmd_list.cmd[cmd_list.size] = 0;
    cmd_list.size += 1;
}

void _sb_cmd_opt(sb_sized_string opt) {
    curr_cmd.length++;
    if (cmd_list.size + opt.size + 5 > cmd_list.cap) {
        cmd_list.cap = cmd_list.cap ? cmd_list.cap * 2 : 256;
        cmd_list.cmd = sb_realloc(cmd_list.cmd, cmd_list.cap);
    }

    cmd_list.cmd[cmd_list.size - 1] = separator;
    cmd_list.cmd[cmd_list.size++] = flag;

    sb_strcpy(&cmd_list.cmd[cmd_list.size], opt);
    cmd_list.size += opt.size;
    cmd_list.cmd[cmd_list.size] = separator;
    cmd_list.size += 1;

}

uint32_t _sb_cmd_arg(sb_sized_string arg) {
    curr_cmd.length++;

    if (cmd_list.size + arg.size + 1 > cmd_list.cap) {
        cmd_list.cap = cmd_list.cap ? cmd_list.cap * 2 : 256;
        cmd_list.cmd = sb_realloc(cmd_list.cmd, cmd_list.cap);
    }

    cmd_list.cmd[cmd_list.size - 1] = separator;

    sb_strcpy(&cmd_list.cmd[cmd_list.size], arg);
    uint32_t index = cmd_list.size;
    cmd_list.size += arg.size;
    cmd_list.cmd[cmd_list.size] = separator;
    cmd_list.size += 1;

    return index;
}

void sb_autobuild(int argc, char* argv[], char* src) {
    //test if should rebuild
    //rebuild based on compiler
    int rebuild = 0; 
    int should_rebuild = sb_cmptime(argv[0], src);
    sb_EXEC() {
        sb_add_file(src);
        sb_set_out(argv[0]);
        sb_add_header("sb.h");

        sb_export_command();
        if (!should_rebuild) {
            sb_printf("No Rebuild\n");
            sb_add_flag("DSB_IMPL");
            sb_dry_run();
            sb_export_command();
        } else {
            rebuild = 1;
            sb_printf("Rebuild\n");
        }
    }

    if (rebuild) {
        sb_build_end();
        execlp(argv[0], argv[0], NULL);
    }
}

void sb_fence() {
    if (cmd_list.isize + 1 > cmd_list.icap) {
        cmd_list.icap = cmd_list.icap ? cmd_list.icap * 2 : 16;
        cmd_list.indicies = sb_realloc(cmd_list.indicies, cmd_list.icap * sizeof(sb_cmd));
    }

    cmd_list.indicies[cmd_list.isize++] = (sb_cmd){
        .start = -1
    };
}

int sb_start_exec() {
    curr_exe.fsize = 0;
    curr_exe.export_commands = 0;
    curr_exe.dry = 0;
    curr_exe.tsize = 0;
    curr_exe.osize = 0;
    curr_exe.hsize = 0;
    curr_exe.incremental = 0;
    curr_exe.output = UINT32_MAX;

    return 0;
}

void _sb_add_header(sb_sized_string f) {
    if (curr_exe.tsize + f.size + 1 > curr_exe.tcap) {
        curr_exe.tcap = curr_exe.tcap ? curr_exe.tcap * 2 : 256;
        curr_exe.text = sb_realloc(curr_exe.text, curr_exe.tcap);
    }

    if (curr_exe.hsize + 1 > curr_exe.hcap) {
        curr_exe.hcap = curr_exe.hcap ? curr_exe.hcap * 2 : 2;
        curr_exe.headers = sb_realloc(curr_exe.headers, curr_exe.hcap * sizeof(uint32_t));
    }

    curr_exe.headers[curr_exe.hsize++] = curr_exe.tsize;
    sb_strcpy(&curr_exe.text[curr_exe.tsize], f);
    curr_exe.tsize += f.size;
    curr_exe.text[curr_exe.tsize] = 0;
    curr_exe.tsize++;
}

void _sb_add_file(sb_sized_string f) {
    if (curr_exe.tsize + f.size + 1 > curr_exe.tcap) {
        curr_exe.tcap = curr_exe.tcap ? curr_exe.tcap * 2 : 256;
        curr_exe.text = sb_realloc(curr_exe.text, curr_exe.tcap);
    }

    if (curr_exe.fsize + 1 > curr_exe.fcap) {
        curr_exe.fcap = curr_exe.fcap ? curr_exe.fcap * 2 : 2;
        curr_exe.files = sb_realloc(curr_exe.files, curr_exe.fcap * sizeof(uint32_t));
    }

    curr_exe.files[curr_exe.fsize++] = curr_exe.tsize;
    sb_strcpy(&curr_exe.text[curr_exe.tsize], f);
    curr_exe.tsize += f.size;
    curr_exe.text[curr_exe.tsize] = 0;
    curr_exe.tsize++;
}

void _sb_add_source_dir(sb_sized_string f) {
    
    DIR* d;
    struct dirent *dir;
    d = opendir(f.string);

    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (sb_cmpext(dir->d_name, "c") == 0) {
                char p[PATH_MAX] = {0};
                snprintf(p, PATH_MAX, "%s/%s", f.string, dir->d_name);
                printf("\t%s\n", p);
                sb_add_file(p);
            }
            if (dir->d_type != DT_DIR) continue;
            if (sb_strcmp(dir->d_name, ".") == 0) continue;
            if (sb_strcmp(dir->d_name, "..") == 0) continue;
            char p[PATH_MAX] = {0};
            snprintf(p, PATH_MAX, "%s/%s", f.string, dir->d_name);
            sb_add_source_dir(p);
        }
        closedir(d);
    } else {
        perror("failed to open file:");
    }
}

void _sb_target_dir(sb_sized_string f) {
    sb_snprintf(build_dir, sizeof(build_dir), "%s", f.string);
}

void _sb_set_out(sb_sized_string f) {
    if (curr_exe.tsize + f.size + 1 > curr_exe.tcap) {
        curr_exe.tcap = curr_exe.tcap ? curr_exe.tcap * 2 : 256;
        curr_exe.text = sb_realloc(curr_exe.text, curr_exe.tcap);
    }

    curr_exe.output = curr_exe.tsize;
    sb_strcpy(&curr_exe.text[curr_exe.tsize], f);
    curr_exe.tsize += f.size;
    curr_exe.text[curr_exe.tsize] = 0;
    curr_exe.tsize++;
}

void _sb_add_flag(sb_sized_string f) {

    if (curr_exe.tsize + f.size + 1 > curr_exe.tcap) {
        curr_exe.tcap = curr_exe.tcap ? curr_exe.tcap * 2 : 2;
        curr_exe.text = sb_realloc(curr_exe.text, curr_exe.tcap);
    }

    if (curr_exe.osize + 1 > curr_exe.ocap) {
        curr_exe.ocap = curr_exe.ocap ? curr_exe.ocap * 2 : 2;
        curr_exe.options = sb_realloc(curr_exe.options, curr_exe.ocap * sizeof(uint32_t));
    }

    curr_exe.options[curr_exe.osize++] = curr_exe.tsize;
    sb_strcpy(&curr_exe.text[curr_exe.tsize], f);
    curr_exe.tsize += f.size;
    curr_exe.text[curr_exe.tsize] = 0;
    curr_exe.tsize++;
}

void sb_set_incremental() {
    curr_exe.incremental = 1;
}

void sb_set_find_deps() {
    curr_exe.auto_deps = 1; 
    sb_add_flag("MP");
    sb_add_flag("MMD");
}

void sb_set_optmize(uint32_t level) {
    switch(level) {
        case 1:
            {
                sb_add_flag("O1");
            } break;
        case 2:
            {
                sb_add_flag("O2");
            } break;
        case 3:
            {
                sb_add_flag("O3");
            } break;
        default: break;
    }
}



//TODO(ELI): Look into automatically enclosing
//path in parentheses to ensure it works with
//spaces
void _sb_add_include_path(sb_sized_string f) {
    char cmd[128] = {0};
    sb_snprintf(cmd, sizeof(cmd), "I%s", f.string);
    sb_add_flag(cmd);
}

void _sb_add_library_path(sb_sized_string f) {
    sb_cmd_opt("-L");
    _sb_cmd_arg(f);
}

void _sb_link_library(sb_sized_string f) {
    char cmd[128] = {0};
    sb_snprintf(cmd, sizeof(cmd), "l%s", f.string);
    sb_add_flag(cmd);
}

void sb_export_command() {
    curr_exe.export_commands = 1;
}

void sb_dry_run() {
    curr_exe.dry = 1;
}

void write_command_entry(char* filep) {
    sb_fprintf(compile_cmds, "{\n");

    sb_fprintf(compile_cmds, "\t\"directory\": \"%s\",\n", sb_get_cwd());
    sb_fprintf(compile_cmds, "\t\"file\": \"%s\",\n", filep);
    sb_fprintf(compile_cmds, "\t\"arguments\": [");

    sb_fprintf(compile_cmds, "\"%.*s\", ", compiler.size, compiler.string);

    //flags            
    for (uint32_t i = 0; i < curr_exe.osize; i++) {
        sb_fprintf(compile_cmds, "\"-%s\", ", &curr_exe.text[curr_exe.options[i]]);
    }

    //sources files
    for (uint32_t i = 0; i < curr_exe.fsize; i++) {
        sb_fprintf(compile_cmds, "\"%s\", ", &curr_exe.text[curr_exe.files[i]]);
    }

    //output
    sb_fprintf(compile_cmds, "\"-o\", ");
    char output[PATH_MAX] = {0};
    sb_snprintf(output, sizeof(output), "\"%s/%s\"", build_dir, &curr_exe.text[curr_exe.output]);
    sb_fprintf(compile_cmds, "%s", output);

    sb_fprintf(compile_cmds, "]\n");
    sb_fprintf(compile_cmds, "}");

}

static int sb_autodeps(char* binname, char* srcname) {
    if (!curr_exe.auto_deps) return 0;
    
    char output[PATH_MAX] = {0};
    sb_snprintf(output, PATH_MAX, "%s.d", srcname);
    sb_printf("Dep File: %s\n", output);

    sb_file f = sb_open(output, sbf_READ, 0);
    if (f == error) {
        return 1; //assume require rebuild
                  //less frustrating to erroneously
                  //rebuild a file rather than fail to rebuild
    }

    int build = 0;
    uint32_t size;
    char* data = sb_mapfile(f, &size);

    char* cursor = data;
    while (cursor[0] != ':') cursor++;
    cursor++;
    while(cursor[0] == ' ') {
        while(cursor[0] != '\n') cursor++;
        cursor++;
    }

    while (cursor - data < size) {
        char* start = cursor;
        while (cursor - data < size && cursor[0] != '\n') {
            cursor++;
        }
        char src[PATH_MAX] = {0};
        snprintf(src, PATH_MAX, "%.*s", (int)((cursor - 1) - start), start);
        build |= sb_cmptime(binname, src);
        cursor++;
    }

    sb_unmapfile(f, data);
    return build;
}


void sb_stop_exec() {

    if (!curr_exe.incremental) {
        sb_CMD() {
            _sb_cmd_main(compiler);
            //flags            
            for (uint32_t i = 0; i < curr_exe.osize; i++) {
                sb_cmd_opt(&curr_exe.text[curr_exe.options[i]]);
            }

            //sources files
            for (uint32_t i = 0; i < curr_exe.fsize; i++) {
                sb_cmd_arg(&curr_exe.text[curr_exe.files[i]]);
            }

            //output
            sb_cmd_opt("o");
            char output[PATH_MAX] = {0};
            sb_snprintf(output, sizeof(output), "%s%s", build_dir, &curr_exe.text[curr_exe.output]);
            sb_cmd_arg(output);
        }
    } else {
        for (uint32_t subfile = 0; subfile < curr_exe.fsize; subfile++) {
            char binname[PATH_MAX] = {0};
            sb_snprintf(binname, sizeof(binname), "%s%s.o", build_dir, sb_basename(&curr_exe.text[curr_exe.files[subfile]]));

            int should_build = sb_cmptime(binname, &curr_exe.text[curr_exe.files[subfile]]);

            char srcname[PATH_MAX] = {0};
            sb_snprintf(srcname, PATH_MAX, "%s%s", build_dir, sb_basename(&curr_exe.text[curr_exe.files[subfile]]));
            should_build |= sb_autodeps(binname, srcname); 

            if (!should_build) continue;
            sb_printf("building: %s\n", binname);
            sb_CMD() {
                _sb_cmd_main(compiler);
                //flags            
                for (uint32_t i = 0; i < curr_exe.osize; i++) {
                    sb_cmd_opt(&curr_exe.text[curr_exe.options[i]]);
                }

                //source file
                sb_cmd_opt("c");
                sb_cmd_arg(&curr_exe.text[curr_exe.files[subfile]]);

                //output
                sb_cmd_opt("o");
                sb_cmd_arg(binname);
            }
        }
        sb_fence();
        int should_build = 0;
        for (uint32_t i = 0; i < curr_exe.fsize; i++) {
            char binname[PATH_MAX] = {0};
            sb_snprintf(binname, sizeof(binname), "%s%s.o", build_dir, sb_basename(&curr_exe.text[curr_exe.files[i]]));
            should_build |= sb_cmptime(binname, &curr_exe.text[curr_exe.files[i]]);
        }

        for (uint32_t i = 0; i < curr_exe.hsize; i++) {
            char binname[PATH_MAX] = {0};
            sb_snprintf(binname, sizeof(binname), "%s%s", build_dir, &curr_exe.text[curr_exe.output]);
            should_build |= sb_cmptime(binname, &curr_exe.text[curr_exe.headers[i]]);
        }

        char binname[PATH_MAX] = {0};
        char srcname[PATH_MAX] = {0};
        sb_snprintf(binname, PATH_MAX, "%s%s", build_dir, &curr_exe.text[curr_exe.output]);
        sb_snprintf(srcname, PATH_MAX, "%s%s", build_dir, &curr_exe.text[curr_exe.output]);
        should_build |= sb_autodeps(binname, srcname); 

        if (should_build) {
            sb_CMD() {
                _sb_cmd_main(compiler);
                //flags            
                for (uint32_t i = 0; i < curr_exe.osize; i++) {
                    sb_cmd_opt(&curr_exe.text[curr_exe.options[i]]);
                }

                //sources files
                for (uint32_t i = 0; i < curr_exe.fsize; i++) {
                    char binname[PATH_MAX] = {0};
                    sb_snprintf(binname, sizeof(binname), "%s%s.o", build_dir, sb_basename(&curr_exe.text[curr_exe.files[i]]));
                    sb_cmd_arg(binname);
                }

                //output
                sb_cmd_opt("o");
                char output[PATH_MAX] = {0};
                sb_snprintf(output, sizeof(output), "%s%s", build_dir, &curr_exe.text[curr_exe.output]);
                sb_cmd_arg(output);
            }
        }
    }


    if (curr_exe.export_commands) {
        //write out compile_commands file
        if (!compile_cmds) {
            compile_cmds = sb_open("compile_commands.json", sbf_WRITE, sbf_CREATE | sbf_TRUNC);
            sb_fprintf(compile_cmds, "[\n");
        } else {
            sb_fprintf(compile_cmds, ",\n");
        }

        //source entries
        for (uint32_t i = 0; i < curr_exe.fsize; i++) {
            write_command_entry(&curr_exe.text[curr_exe.files[i]]);
            sb_fprintf(compile_cmds, ",\n");
        }

        //header entries
        for (uint32_t i = 0; i < curr_exe.hsize; i++) {
            write_command_entry(&curr_exe.text[curr_exe.headers[i]]);
            if (i + 1 < curr_exe.hsize) {
                sb_fprintf(compile_cmds, ",\n");
            }
        }

    }

    curr_exe.fsize = 0;
    curr_exe.export_commands = 0;
    curr_exe.dry = 0;
    curr_exe.tsize = 0;
    curr_exe.osize = 0;
    curr_exe.hsize = 0;
    curr_exe.incremental = 0;
    curr_exe.output = UINT32_MAX;
}



#endif
#endif
