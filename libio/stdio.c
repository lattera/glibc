#include "libioP.h"
#include "stdio.h"

/* Define non-macro versions of stdin/stdout/stderr,
 * for use by debuggers. */

#undef stdin
#undef stdout
#undef stderr
FILE* stdin = &_IO_stdin_.file;
FILE* stdout = &_IO_stdout_.file;
FILE* stderr = &_IO_stderr_.file;
