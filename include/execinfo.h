#ifndef _EXECINFO_H
#include <debug/execinfo.h>

extern int __backtrace (void **__array, int __size);

extern char **__backtrace_symbols (void *__const *__array, int __size);

extern void __backtrace_symbols_fd (void *__const *__array, int __size,
				    int __fd);

#endif
