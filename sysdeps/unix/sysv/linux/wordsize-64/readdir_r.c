#define readdir64_r __no_readdir64_r_decl
#include <sysdeps/unix/readdir_r.c>
#undef readdir64_r
weak_alias (__readdir_r, readdir64_r)
