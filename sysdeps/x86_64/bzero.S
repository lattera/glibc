#define memset __bzero
#include <sysdeps/x86_64/memset.S>
weak_alias (__bzero, bzero)
