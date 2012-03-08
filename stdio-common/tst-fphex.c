/* Test program for %a printf formats.  */

#include <stdio.h>
#include <string.h>

#ifndef STR_LEN
# define STR_LEN strlen
#endif
#ifndef STR_CMP
# define STR_CMP strcmp
#endif
#ifndef SPRINT
# define SPRINT snprintf
#endif
#ifndef CHAR_T
# define CHAR_T char
#endif
#ifndef PRINT
# define PRINT printf
#endif
#ifndef L_
# define L_(Str) Str
#endif
#ifndef L
# define L
#endif

struct testcase
{
  double value;
  const CHAR_T *fmt;
  const CHAR_T *expect;
};

static const struct testcase testcases[] =
  {
    { 0x0.0030p+0, L_("%a"),		L_("0x1.8p-11") },
    { 0x0.0040p+0, L_("%a"),		L_("0x1p-10") },
    { 0x0.0030p+0, L_("%040a"),		L_("0x00000000000000000000000000000001.8p-11") },
    { 0x0.0040p+0, L_("%040a"),		L_("0x0000000000000000000000000000000001p-10") },
    { 0x0.0040p+0, L_("%40a"),		L_("                                 0x1p-10") },
    { 0x0.0040p+0, L_("%#40a"),		L_("                                0x1.p-10") },
    { 0x0.0040p+0, L_("%-40a"),		L_("0x1p-10                                 ") },
    { 0x0.0040p+0, L_("%#-40a"),	L_("0x1.p-10                                ") },
    { 0x0.0030p+0, L_("%040e"),		L_("00000000000000000000000000007.324219e-04") },
    { 0x0.0040p+0, L_("%040e"),		L_("00000000000000000000000000009.765625e-04") },
  };


static int
do_test (void)
{
  const struct testcase *t;
  int result = 0;

  for (t = testcases;
       t < &testcases[sizeof testcases / sizeof testcases[0]];
       ++t)
    {
      CHAR_T buf[1024];
      int n = SPRINT (buf, sizeof buf / sizeof (buf[0]), t->fmt, t->value);
      if (n != STR_LEN (t->expect) || STR_CMP (buf, t->expect) != 0)
	{
	  PRINT (L_("%" L "s\tExpected \"%" L "s\" (%Zu)\n\tGot      \"%" L
		 "s\" (%d, %Zu)\n"), t->fmt, t->expect, STR_LEN (t->expect),
		 buf, n, STR_LEN (buf));
	  result = 1;
	}
    }

  return result;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
