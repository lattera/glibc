#define USE_AS_STPCPY
#define STRCPY __stpcpy

#include <sysdeps/i386/i586/strcpy.S>

weak_alias (__stpcpy, stpcpy)
