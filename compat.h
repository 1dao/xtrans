#ifndef _COMPAT_H_
#define _COMPAT_H_

#ifdef _WIN32
#include <stdio.h>
#include <string.h>
#include <windows.h>

// getopt_long implementation for Windows
struct option {
    const char *name;
    int has_arg;
    int *flag;
    int val;
};

#define no_argument 0
#define required_argument 1
#define optional_argument 2

// Global variables - defined only once using header guard
#ifndef COMPAT_VARS_DEFINED
#define COMPAT_VARS_DEFINED
char *optarg = NULL;
int optind = 1;
int opterr = 1;
int optopt = 0;
#endif

// Inline function implementations
static inline int getopt_internal(int argc, char * const argv[], const char *optstring,
                                 const struct option *longopts, int *longindex, int long_only) {
    static int pos = 1;

    if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0') {
        return -1;
    }

    // Handle long options
    if (longopts && (argv[optind][1] == '-' || long_only)) {
        const char *name = argv[optind] + (argv[optind][1] == '-' ? 2 : 1);
        const struct option *opt;

        for (opt = longopts; opt->name; opt++) {
            size_t len = strlen(opt->name);
            if (strncmp(name, opt->name, len) == 0) {
                if (name[len] == '\0') {
                    // Exact match
                    if (opt->has_arg == required_argument) {
                        if (optind + 1 >= argc) {
                            if (opterr) {
                                fprintf(stderr, "%s: option '--%s' requires an argument\n", argv[0], opt->name);
                            }
                            return '?';
                        }
                        optarg = argv[++optind];
                    } else {
                        optarg = NULL;
                    }
                    optind++;
                    pos = 1;
                    if (longindex) *longindex = (int)(opt - longopts);
                    return opt->val;
                } else if (name[len] == '=') {
                    // Option with argument using =
                    if (opt->has_arg == required_argument) {
                        optarg = (char*)(name + len + 1);
                        optind++;
                        pos = 1;
                        if (longindex) *longindex = (int)(opt - longopts);
                        return opt->val;
                    } else {
                        if (opterr) {
                            fprintf(stderr, "%s: option '--%s' doesn't allow an argument\n", argv[0], opt->name);
                        }
                        return '?';
                    }
                }
            }
        }

        if (opterr) {
            fprintf(stderr, "%s: unrecognized option '%s'\n", argv[0], argv[optind]);
        }
        optind++;
        pos = 1;
        return '?';
    }

    // Handle short options
    if (pos == 1) {
        pos++;
    }

    char opt_char = argv[optind][pos];
    const char *p = strchr(optstring, opt_char);

    if (opt_char == '\0' || p == NULL) {
        if (opterr) {
            fprintf(stderr, "%s: illegal option -- %c\n", argv[0], opt_char);
        }
        optopt = opt_char;
        return '?';
    }

    if (*(p + 1) == ':') {
        if (argv[optind][pos + 1] != '\0') {
            optarg = (char*)&argv[optind][pos + 1];
            optind++;
            pos = 1;
        } else if (++optind < argc) {
            optarg = argv[optind];
            optind++;
            pos = 1;
        } else {
            if (opterr) {
                fprintf(stderr, "%s: option requires an argument -- %c\n", argv[0], opt_char);
            }
            optopt = opt_char;
            optarg = NULL;
            return '?';
        }
    } else {
        if (argv[optind][pos + 1] != '\0') {
            pos++;
        } else {
            optind++;
            pos = 1;
        }
        optarg = NULL;
    }

    return opt_char;
}

static inline int getopt(int argc, char * const argv[], const char *optstring) {
    return getopt_internal(argc, argv, optstring, NULL, NULL, 0);
}

static inline int getopt_long(int argc, char * const argv[], const char *optstring,
                             const struct option *longopts, int *longindex) {
    return getopt_internal(argc, argv, optstring, longopts, longindex, 0);
}

static void console_set_font_to_consolas() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        return;
    }

    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    if (!GetCurrentConsoleFontEx(hConsole, FALSE, &cfi)) {
        return;
    }

    wcscpy_s(cfi.FaceName, LF_FACESIZE, L"Consolas");
    cfi.dwFontSize.X = 12;
    cfi.dwFontSize.Y = 24;
    cfi.FontWeight = FW_NORMAL;

    SetCurrentConsoleFontEx(hConsole, FALSE, &cfi);
}

#else
// Unix/Linux
#include <unistd.h>
#include <getopt.h>
#endif

#endif /* _COMPAT_H_ */
