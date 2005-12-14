#define glob64 __no_glob64_decl
#define globfree64 __no_globfree64_decl
#include <posix/glob.c>
#undef glob64
#undef globfree64
weak_alias (glob, glob64)
weak_alias (globfree, globfree64)
libc_hidden_ver (globfree, globfree64)
