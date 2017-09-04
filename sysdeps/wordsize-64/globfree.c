#define globfree64 __no_globfree64_decl
#include <posix/globfree.c>
#undef globfree64
weak_alias (globfree, globfree64)
libc_hidden_ver (globfree, globfree64)
