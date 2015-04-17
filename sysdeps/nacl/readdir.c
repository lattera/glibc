/* The compiler complains about aliases with nonmatching type signatures.
   The types 'struct dirent' and 'struct dirent64' are actually identical
   even though the compiler doesn't consider them to be.  So we hide the
   declaration from the compiler.  */
#define __readdir64     __avoid___readdir64_declaration
#define readdir64       __avoid_readdir64_declaration
#include <sysdeps/posix/readdir.c>
#undef  __readdir64
#undef  readdir64
strong_alias (__readdir, __readdir64)
weak_alias (__readdir64, readdir64)
