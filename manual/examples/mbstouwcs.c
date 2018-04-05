#include <stdlib.h>
#include <string.h>
#include <wchar.h>

/* Do not include the above headers in the example.
*/
wchar_t *
mbstouwcs (const char *s)
{
  size_t len = strlen (s);
  wchar_t *result = malloc ((len + 1) * sizeof (wchar_t));
  wchar_t *wcp = result;
  wchar_t tmp[1];
  mbstate_t state;
  size_t nbytes;

  memset (&state, '\0', sizeof (state));
  while ((nbytes = mbrtowc (tmp, s, len, &state)) > 0)
    {
      if (nbytes >= (size_t) -2)
        /* Invalid input string.  */
        return NULL;
      *wcp++ = towupper (tmp[0]);
      len -= nbytes;
      s += nbytes;
    }
  return result;
}
