#define _GNU_SOURCE 1
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/param.h>

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

int
main (int argc, char *argv[])
{
  size_t size = sysconf (_SC_PAGESIZE);
  char *adr;
  int result = 0;

  adr = (char *) mmap (NULL, size, PROT_READ|PROT_WRITE,
                       MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  if (adr == NULL)
    {
      if (errno == ENOSYS)
        puts ("No test, mmap not available.");
      else
        {
          printf ("mmaping failed: %m");
          result = 1;
        }
    }
  else
    {
      char dest[size];
      int inner, middle, outer;

      memset (adr, 'T', size);

      /* strlen test */
      for (outer = size - 1; outer >= MAX (0, size - 128); --outer)
        {
          for (inner = MAX (outer, size - 64); inner < size; ++inner)
	    {
	      adr[inner] = '\0';

	      if (strlen (&adr[outer]) != inner - outer)
		{
		  printf ("strlen flunked for outer = %d, inner = %d\n",
			  outer, inner);
		  result = 1;
		}

	      adr[inner] = 'T';
	    }
        }

      /* strchr test */
      for (outer = size - 1; outer >= MAX (0, size - 128); --outer)
        {
	  for (middle = MAX (outer, size - 64); middle < size; ++middle)
	    {
	      for (inner = middle; inner < size; ++inner)
		{
		  char *cp;
		  adr[middle] = 'V';
		  adr[inner] = '\0';

		  cp = strchr (&adr[outer], 'V');

		  if ((inner == middle && cp != NULL)
		      || (inner != middle
			  && (cp - &adr[outer]) != middle - outer))
		    {
		      printf ("strchr flunked for outer = %d, middle = %d, "
			      "inner = %d\n", outer, middle, inner);
		      result = 1;
		    }

		  adr[inner] = 'T';
		  adr[middle] = 'T';
		}
	    }
        }

      /* strrchr test */
      for (outer = size - 1; outer >= MAX (0, size - 128); --outer)
        {
	  for (middle = MAX (outer, size - 64); middle < size; ++middle)
	    {
	      for (inner = middle; inner < size; ++inner)
		{
		  char *cp;
		  adr[middle] = 'V';
		  adr[inner] = '\0';

		  cp = strrchr (&adr[outer], 'V');

		  if ((inner == middle && cp != NULL)
		      || (inner != middle
			  && (cp - &adr[outer]) != middle - outer))
		    {
		      printf ("strrchr flunked for outer = %d, middle = %d, "
			      "inner = %d\n", outer, middle, inner);
		      result = 1;
		    }

		  adr[inner] = 'T';
		  adr[middle] = 'T';
		}
	    }
        }

      /* strcpy test */
      for (outer = size - 1; outer >= MAX (0, size - 128); --outer)
        {
          for (inner = MAX (outer, size - 64); inner < size; ++inner)
	    {
	      adr[inner] = '\0';

	      if (strcpy (dest, &adr[outer]) != dest
		  || strlen (dest) != inner - outer)
		{
		  printf ("strcpy flunked for outer = %d, inner = %d\n",
			  outer, inner);
		  result = 1;
		}

	      adr[inner] = 'T';
	    }
        }

      /* stpcpy test */
      for (outer = size - 1; outer >= MAX (0, size - 128); --outer)
        {
          for (inner = MAX (outer, size - 64); inner < size; ++inner)
	    {
	      adr[inner] = '\0';

	      if ((stpcpy (dest, &adr[outer]) - dest) != inner - outer)
		{
		  printf ("stpcpy flunked for outer = %d, inner = %d\n",
			  outer, inner);
		  result = 1;
		}

	      adr[inner] = 'T';
	    }
        }
    }

  return result;
}
