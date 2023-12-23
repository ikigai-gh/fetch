#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/utsname.h>
#include <assert.h>
#include <errno.h>
#ifdef X11
#include <X11/Xlib.h>
#endif

#include "lib.h"

// TODO: Figure out better buffer size
#define SIZE 64
#define STR_NULL_CHAR 1
#define ARR_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

// Low-level functions for reading files
static char *find_str(const char *path, const char *str);
static char *read_str(const char *path);

// My secure implementation of string functions from libc
static inline char *f_strcpy(char *dst, const char *src, size_t len);
static inline char *f_strcat(char *dst, const char *src, size_t len);

static char *find_str(const char* path, const char* str)
{
    FILE *fd = fopen(path, "r");

    if (!fd)
    {
        fprintf(stderr, "fopen() error %s: %s\n", path, strerror(errno));
        return NULL;
    }

    char buf[SIZE];
    char *str_ptr;

    while (fgets(buf, SIZE, fd))
    {
        if (strstr(buf, str))
        {
            str_ptr = calloc(strlen(buf) + STR_NULL_CHAR, sizeof(char));
            if (!str_ptr)
            {
                perror("calloc");
                return NULL;
            }
            f_strcpy(str_ptr, buf, ARR_SIZE(buf));
            return str_ptr;
        }
    }

    fclose(fd);
    return NULL;
}
static char *read_str(const char* path)
{
    FILE *fd = fopen(path, "r");

    if (!fd)
    {
        fprintf(stderr, "fopen() error %s: %s\n", path, strerror(errno));
        return NULL;
    }

    char *buf = calloc(sizeof(char) * SIZE, sizeof(char));

    if (!buf)
    {
        perror("calloc");
        return NULL;
    }

    char *str = fgets(buf, SIZE, fd);

    if (!str)
    {
        perror("fgets");
        return NULL;
    }

    fclose(fd);

    return buf;
}

static void del_ch(char *str, char ch, size_t del_count)
{
    size_t deleted = 0;
    char *rptr = str, *wptr = str;

    while (*rptr != '\0')
    {
        if (*rptr == ch)
        {
            if (deleted != del_count)
            {
                rptr++;
                deleted++;
            }
        }
        *wptr = *rptr;
        rptr++;
        wptr++;
    }
    *wptr = '\0';
}

static char *try_files(const char *files[], size_t n)
{
    char *content = NULL;
    size_t i = 0;

    while (content == NULL && i != n)
    {
        content = read_str(files[i]);
        i++;
    }

    return content;
}

static inline char *f_strcpy(char *dst, const char *src, size_t len)
{
    assert(strlen(src) + STR_NULL_CHAR <= len);
    strcpy(dst, src);
    return dst;
}

static inline char *f_strcat(char *dst, const char *src, size_t len)
{
    assert(strlen(src) + STR_NULL_CHAR <= len);
    strcat(dst, src);
    return dst;
}

char *fetch_os()
{
    char *os = NULL;
    const char *OS_INFO_FILE = "/etc/os-release";
    const char *OS_INFO_SEARCH_STR = "PRETTY_NAME";

    char *os_str = find_str(OS_INFO_FILE, OS_INFO_SEARCH_STR);

    if (!os_str)
    {
        size_t sz = strlen("Unknown") + STR_NULL_CHAR;
        char *os = calloc(sz, sizeof(char));
        f_strcpy(os, "Unknown", sz);
        return os;
    }

    strtok(os_str, "=");
    char *token = strtok(NULL, "=");
    char tok_buf[SIZE];
    f_strcpy(tok_buf, token, ARR_SIZE(tok_buf));

    if (!token)
    {
        goto error;
    }

    del_ch(tok_buf, '\n', 1);
    del_ch(tok_buf, '\"', 2);

    // TODO: Figure out not to call uname() twice
    struct utsname info;

    if (uname(&info) == -1)
    {
        perror("uname");
        goto error;
    }

    f_strcat(tok_buf, " ", ARR_SIZE(tok_buf));
    size_t os_sz = strlen(tok_buf) + strlen(info.machine) + STR_NULL_CHAR;
    os = calloc(os_sz, sizeof(char));

    if (!os)
    {
        perror("calloc");
        goto error;
    }

    f_strcpy(os, f_strcat(tok_buf, info.machine, ARR_SIZE(tok_buf)), os_sz);

    free(os_str);

    return os;

error:
    free(os_str);
    return NULL;
}

char *fetch_host()
{
    char *host = NULL;
    size_t h_sz = 0;

    const char *PRODUCT_INFO_FILES[] =
    {
        "/sys/class/dmi/id/product_name",
        "/sys/firmware/devicetree/base/model"
    };

    char *product = try_files(PRODUCT_INFO_FILES, 2);

    if (product != NULL)
    {
        del_ch(product, '\n', 1);
    }

    else
    {
        h_sz = strlen("Unknown") + STR_NULL_CHAR;
        char *host = calloc(h_sz, sizeof(char));
        f_strcpy(host, "Unknown", h_sz);
        return host;
    }

    const char *VERSION_INFO_FILE = "/sys/class/dmi/id/product_version";
    char *version = read_str(VERSION_INFO_FILE);

    if (version != NULL)
    {
        del_ch(version, '\n', 1);
    }

    f_strcat(product, " ", SIZE);
    size_t host_sz = strlen(product) + strlen(version) + STR_NULL_CHAR;

    host = calloc(host_sz, sizeof(char));
    f_strcpy(host, f_strcat(product, version, host_sz), host_sz);

    free(product);
    free(version);

    return host;
}

char *fetch_kern()
{
    struct utsname info;

    if (uname(&info) == -1)
    {
        perror("uname");
        return NULL;
    }

    const size_t kern_sz = strlen(info.release) + STR_NULL_CHAR;
    char *kern = calloc(kern_sz, sizeof(char));

    if (!kern)
    {
        perror("calloc");
        return NULL;
    }

    f_strcpy(kern, info.release, kern_sz);

    return kern;
}

char *fetch_uptime()
{
    const char *UPTIME_INFO_FILE = "/proc/uptime";
    char *uptime_str = read_str(UPTIME_INFO_FILE);
    size_t seconds = 0;

    if (uptime_str)
    {
        char *token = strtok(uptime_str, " ");
        seconds = (size_t)atof(token);
    }

    size_t years = 0, days = 0, hours = 0, minutes = 0;

    years = seconds / (3600 * 24 * 365);
    seconds -= 3600 * 24 * 365 * years;
    days = seconds / (3600 * 24);
    seconds -= 3600 * 24 * days;
    hours = seconds / 3600;
    seconds -= 3600 * hours;
    minutes = seconds / 60;
    seconds -= 60 * minutes;

    // FIXME: Hardcoded size
    char uptime_fmt[SIZE];
    snprintf(uptime_fmt, SIZE, "%zuy %zud %zuh %zum", years, days, hours, minutes);
    const size_t up_sz = SIZE + STR_NULL_CHAR;

    char *uptime = calloc(up_sz, sizeof(char));

    if (!uptime)
    {
        perror("calloc");
        free(uptime_str);
        return NULL;
    }

    f_strcpy(uptime, uptime_fmt, up_sz);
    free(uptime_str);

    return uptime;
}

char *fetch_cpu()
{
    const char *CPU_INFO_FILE = "/proc/cpuinfo";
    const char *CPU_INFO_STRS[] = {"model name", "Processor"};
    char *cpu_str = NULL;
    size_t cpu_sz = 0;

    for (size_t i = 0; i < sizeof(CPU_INFO_STRS) / sizeof(CPU_INFO_STRS[0]); i++)
    {
        cpu_str = find_str(CPU_INFO_FILE, CPU_INFO_STRS[i]);

        if (cpu_str)
        {
            break;
        }
    }

    if (!cpu_str)
    {
        cpu_sz = strlen("Unknown") + STR_NULL_CHAR;
        char *cpu = calloc(cpu_sz, sizeof(char));
        f_strcpy(cpu, "Unknown", cpu_sz);
        return cpu;
    }

    // Ignore first token
    strtok(cpu_str, ":");
    char *token = strtok(NULL, ":");

    if (!token)
    {
        free(cpu_str);
        return NULL;
    }

    cpu_sz = strlen(token) + STR_NULL_CHAR;
    char *cpu = calloc(cpu_sz, sizeof(char));

    if (!cpu)
    {
        perror("calloc");
        free(cpu_str);
        return NULL;
    }

    f_strcpy(cpu, token, cpu_sz);

    if (cpu)
    {
        del_ch(cpu, ' ', 1);
        del_ch(cpu, '\n', 1);
    }

    free(cpu_str);

    return cpu;
}

char *fetch_shell()
{
    const char *shells[] = {"sh", "bash", "zsh", "csh", "tsch", "ksh", "ash", "fish"};

    char *shell_str = getenv("SHELL");
    size_t sh_sz = 0;

    if (!shell_str)
    {
        return NULL;
    }

    char *shell_token = strtok(strrchr(shell_str, '/'), "/");

    if (!shell_token)
    {
        return NULL;
    }

    bool shell_recognized = false;

    for (size_t i = 0; i < sizeof(shells) / sizeof(shells[0]); i++)
    {
        if (!strcmp(shell_token, shells[i]))
        {
            shell_recognized = true;
            break;
        }
    }

    if (!shell_recognized)
    {
        sh_sz = strlen("Unknown") + STR_NULL_CHAR;
        char *unknown_shell = calloc(sh_sz, sizeof(char));
        f_strcpy(unknown_shell, "Unknown", sh_sz);
        return unknown_shell;
    }

    sh_sz = strlen(shell_token) + STR_NULL_CHAR;
    char *shell = calloc(sh_sz, sizeof(char));

    if (!shell)
    {
        perror("calloc");
        return NULL;
    }

    f_strcpy(shell, shell_token, sh_sz);

    return shell;
}

#ifdef X11
char *fetch_res()
{
    Display *dpy = XOpenDisplay(0);

    if (!dpy)
    {
        return NULL;
    }

    int scr_w = XDisplayWidth(dpy, 0);
    int scr_h = XDisplayHeight(dpy, 0);

    // FIXME: Hardcoded size
    char res_fmt[SIZE];
    sprintf(res_fmt, "%dx%d", scr_w, scr_h);

    char *res = calloc(strlen(res_fmt) + STR_NULL_CHAR, sizeof(char));

    if (!res)
    {
        perror("calloc");
        return NULL;
    }

    f_strcpy(res, res_fmt);

    return res;
}
#endif
