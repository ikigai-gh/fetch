#include "lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>

int main(void)
{
    printf("fetch-%s\n", VERSION);
    printf("=====================\n");
    char *os = fetch_os();

    if (os)
    {
        printf("OS: %s\n", os);
        free(os);
    }

    char *host = fetch_host();

    if (host)
    {
        printf("Host: %s\n", host);
        free(host);
    }

    char *kern = fetch_kern();

    if (kern)
    {
        printf("Kern: %s\n", kern);
        free(kern);
    }

    char *uptime = fetch_uptime();

    if (uptime)
    {
        printf("Uptime: %s\n", uptime);
        free(uptime);
    }

    char *shell = fetch_shell();

    if (shell)
    {
        printf("Shell: %s\n", shell);
        free(shell);
    }

#ifdef X11
    char *res = fetch_res();

    if (res)
    {
        printf("Res: %s\n", res);
        free(res);
    }
#endif

    char *cpu = fetch_cpu();

    if (cpu)
    {
        printf("CPU: %s\n", cpu);
        free(cpu);
    }

    return 0;
}
