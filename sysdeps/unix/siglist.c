#include <ansidecl.h>
#include <stddef.h>

#ifndef HAVE_GNU_LD
#define _sys_siglist    sys_siglist
#endif

/* This is a list of all known signal numbers.  */

CONST char *CONST _sys_siglist[] =
  {
    "Signal 0",
    "Hangup",
    "Interrupt",
    "Quit",
    "Illegal instruction",
    "Trace/BPT trap",
    "IOT trap",
    "EMT trap",
    "Floating point exception",
    "Killed",
    "Bus error",
    "Segmentation fault",
    "Bad system call",
    "Broken pipe",
    "Alarm clock",
    "Terminated",
    "Urgent I/O condition",
    "Stopped (signal)",
    "Stopped",
    "Continued",
    "Child exited",
    "Stopped (tty input)",
    "Stopped (tty output)",
    "I/O possible",
    "Cputime limit exceeded",
    "Filesize limit exceeded",
    "Virtual timer expired",
    "Profiling timer expired",
    "Window changed",
    "Resource lost",
    "User defined signal 1",
    "User defined signal 2",
    NULL
  };

