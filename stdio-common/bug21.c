#include <stdio.h>
#include <libc-diag.h>

static int
do_test (void)
{
  static const char buf[] = " ";
  char *str;

  /* GCC in C99 mode treats %a as the C99 format expecting float *,
     but glibc with _GNU_SOURCE treats %as as the GNU allocation
     extension, so resulting in "warning: format '%a' expects argument
     of type 'float *', but argument 3 has type 'char **'".  This
     applies to the other %as, %aS and %a[] formats below as well.  */
  DIAG_PUSH_NEEDS_COMMENT;
  DIAG_IGNORE_NEEDS_COMMENT (4.9, "-Wformat");
  int r = sscanf (buf, "%as", &str);
  DIAG_POP_NEEDS_COMMENT;
  printf ("%d %p\n", r, str);

  return r != -1 || str != NULL;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
