#include <stdio.h>
#include <string.h>

int main (int argc, char** argv)
{
  const char haystack[] =
    "F_BD_CE_BD_EF_BF_BD_EF_BF_BD_EF_BF_BD_EF_BF_BD_C3_88_20_EF_BF_BD_EF_BF_BD_EF_BF_BD_C3_A7_20_EF_BF_BD";

  const char needle[] =
    "_EF_BF_BD_EF_BF_BD_EF_BF_BD_EF_BF_BD_EF_BF_BD";

  const char* sub = strstr (haystack, needle);

  if (sub != NULL)
    {
      int j;

      fprintf (stderr, "BUG: expected NULL, got:\n%s\n%s\n", sub, needle);
      for (j = 0; needle[j] != '\0'; ++j)
        putchar (needle[j] == sub[j] ? ' ' : '^');
      puts ("");
      return 1;
    }

  return 0;
}
