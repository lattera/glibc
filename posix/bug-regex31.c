#include <mcheck.h>
#include <regex.h>
#include <stdio.h>
#include <sys/types.h>

int main()
{
 regex_t regex;
 int rc;

  mtrace ();

 if ((rc = regcomp (&regex, "([0]", REG_EXTENDED)))
   printf ("Error %d (expected)\n", rc);
 return 0;
}
