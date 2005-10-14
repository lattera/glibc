#include <stddef.h>
#include <stdio.h>
#include <sys/ucontext.h>

#include <ucontext_i.h>

static int
do_test  (void)
{
  int nerrors = 0;
  int ntests = 0;

#define TEST(member, val) \
  do {									\
    if (offsetof (struct ucontext, member) != val)			\
      {									\
	printf ("offsetof(%s) = %zu, %s = %zu\n",			\
		#member, offsetof (struct ucontext, member),		\
		#val, (size_t) val);					\
	++nerrors;							\
      }									\
    ++ntests;								\
  } while (0)

#ifdef TESTS
  TESTS
#endif

  printf ("%d errors in %d tests\n", nerrors, ntests);

  return nerrors != 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
