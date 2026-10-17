/* host.h - private interface between host.c and host_z80.c */
#ifndef HOST_H
#define HOST_H

#include <stdint.h>

extern int host_typeahead;

/* Run a native Z80 system image (does not return) */
void host_run_z80(const char *image) __attribute__((noreturn));

#endif
