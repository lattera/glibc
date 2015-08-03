#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

static int
do_test (void)
{
  printf( "main\n" );
  exit(EXIT_SUCCESS);
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
