#include <sys/types.h>
#include <regex.h>
#include <locale.h>
#include <stdio.h>

int
main (int argc, char *argv[])
{
  regex_t re;
  regmatch_t mat[1];
  int res = 1;

  if (setlocale (LC_ALL, "de_DE.ISO-8859-1") == NULL)
    puts ("cannot set locale");
  else if (regcomp (&re, "[a-f]*", 0) != REG_NOERROR)
    puts ("cannot compile expression \"[a-f]*\"");
  else if (regexec (&re, "abcdefCDEF", 1, mat, 0) == REG_NOMATCH)
    puts ("no match");
  else
    {
      printf ("match from %d to %d\n", mat[0].rm_so, mat[0].rm_eo);
      res = mat[0].rm_so != 0 || mat[0].rm_eo != 6;
    }

  return res;
}
