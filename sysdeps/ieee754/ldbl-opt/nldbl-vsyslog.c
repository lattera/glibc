#include "nldbl-compat.h"

void
vsyslog (int pri, const char *fmt, va_list ap)
{
  __nldbl_vsyslog (pri, fmt, ap);
}
