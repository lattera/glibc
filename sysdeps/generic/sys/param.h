/* This file should contain various parameter macros appropriate for the
   machine and operating system.  There is no standard set of macros; this
   file is just for compatibility with programs written for Unix that
   expect it to define things.  On Unix systems that do not have their own
   sysdep version of this file, it is generated at build time by examining
   the installed headers on the system.  */

#include <limits.h>

#define MAXSYMLINKS  1
#define MAXPATHLEN   256

/* Macros for min/max.  */
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
