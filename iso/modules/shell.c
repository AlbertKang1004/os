#include "ulib.h"

struct command { 
    const char *name; 
    int (*fn)(int, char **); 
};

static int echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        write(1, argv[i], strlen(argv[i]));
        puts(" ");
    }
    return 0;
}

static int cat(int argc, char **argv) {
    if (argc < 2) { 
        write(SYSERR_FILENO, "usage: cat <filename>", 21);
        return 2; // invalid argument
    }
    int fd, n;
    char buf[128];
    fd = open(argv[1], 0, 0);
    if (fd == -1) {
        write(SYSERR_FILENO, "cat: file \"", 11);
        write(SYSERR_FILENO, argv[1], strlen(argv[1]));
        write(SYSERR_FILENO, "\" not found.", 12);
        return 1; // no file exist
    }
    while (1) {
        if ((n = read(fd, buf, 128)) < 1) {
            break;
        }
        write(SYSOUT_FILENO, buf, n);
    }
    close(fd);
    return 0;
}

static int ls(int argc, char **argv) {
    
}

static struct command com[] = {
    { .name = "echo",   .fn = echo},
    { .name = "cat",    .fn = cat},
    { .name = "ls",     .fn = ls}
};

static int tokenize(char *line, char **argv, int max_args) {
    char * cur = line;
    int arg_count = 0;
    while (arg_count < max_args) {
        while (*cur == ' ') cur++; // skip preceding spaces
        if (*cur == 0) return arg_count;
        argv[arg_count++] = cur;
        while (*cur != ' ' && *cur != 0) cur++; // skip existing token
        if (*cur == ' ') *(cur++) = 0;
    }
    return arg_count;
}

static int execute(int argc, char **argv) {
    write(1, "\n", 1);
    for (unsigned int i = 0; i < sizeof(com) / sizeof(com[0]); i++) {
        if (strcmp(com[i].name, argv[0]) == 0) {
            com[i].fn(argc, argv); 
            return 0;
        }
    }
    write(SYSERR_FILENO, "command not found", 17);
    return -1; // command not found
}

int main(void) {
    char buf[1];
    char line[128];
    char *argv[8];
    int len = 0;
    int argc;
    puts("$ ");
    while (1) {
        read(0, buf, 1);
        switch (*buf) {
            case '\n':
                line[len] = 0;
                argc = tokenize(line, argv, 8);
                if (argc >= 1) execute(argc, argv);                    
                len = 0;
                puts("\n$ ");
                break;
            case '\b':
                if (len > 0) {
                    puts("\b \b");
                    len--;
                }
                break;
            default:
                if (len < 127 && *buf >= 0x20 && *buf <= 0x7E) {
                    write(SYSOUT_FILENO, buf, 1);
                    line[len++] = *buf;
                }
        }
    }
    exit(0);
}