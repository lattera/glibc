#ifndef __ASSEMBLY__
# include <stdint.h>
extern void _start (uint32_t info[]) attribute_hidden;
#endif

#define ENTRY_POINT _start
