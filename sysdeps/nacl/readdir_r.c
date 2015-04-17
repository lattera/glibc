/* The compiler complains about aliases with nonmatching type signatures.
   The types 'struct dirent' and 'struct dirent64' are actually identical
   even though the compiler doesn't consider them to be.  So we hide the
   declaration from the compiler.  */
#define readdir64_r     __avoid_readdir64_r_declaration
#include <sysdeps/posix/readdir_r.c>
#undef  readdir64_r
weak_alias (__readdir_r, readdir64_r)
