#define readdir64 __no_readdir64_decl
#include <sysdeps/unix/readdir.c>
#undef readdir64
weak_alias (__readdir, readdir64)
