#define NO_SHLIB
/* Solaris needs start named `_start', not `start'.  */
#define NO_EXPLICIT_START
#include <sysdeps/unix/sparc/start.c>
