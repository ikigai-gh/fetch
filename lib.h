#ifndef _LIB_H
#define _LIB_H

// Public API
char *fetch_os();
char *fetch_host();
char *fetch_kern();
char *fetch_uptime();
char *fetch_cpu();
char *fetch_shell();
#ifdef X11
char *fetch_res();
#endif

#endif
