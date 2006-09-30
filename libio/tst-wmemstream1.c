#include <wchar.h>

#define CHAR_T wchar_t
#define W(o) L##o
#define OPEN_MEMSTREAM open_wmemstream

#include "tst-memstream1.c"
