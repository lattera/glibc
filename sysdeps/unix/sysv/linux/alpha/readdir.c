#define readdir64 __no_readdir64_decl
#define __readdir64 __no___readdir64_decl
#include <sysdeps/unix/readdir.c>
#undef __readdir64
strong_alias (__readdir, __readdir64)
#undef readdir64
weak_alias (__readdir, readdir64)
