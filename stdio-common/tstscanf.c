/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#ifdef	BSD
#include </usr/include/stdio.h>
#else
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>


int
DEFUN(main, (argc, argv), int argc AND char **argv)
{
  char buf[BUFSIZ];
  FILE *in = stdin, *out = stdout;

  if (argc == 2 && !strcmp (argv[1], "-opipe"))
    {
      out = popen ("/bin/cat", "w");
      if (out == NULL)
	{
	  perror ("popen: /bin/cat");
	  exit (EXIT_FAILURE);
	}
    }
  else if (argc == 3 && !strcmp (argv[1], "-ipipe"))
    {
      sprintf (buf, "/bin/cat %s", argv[2]);
      in = popen (buf, "r");
    }

  {
    char name[50];
    fprintf (out,
	     "sscanf (\"thompson\", \"%%s\", name) == %d, name == \"%s\"\n",
	     sscanf ("thompson", "%s", name),
	     name);
  }

  fputs ("Testing scanf (vfscanf)\n", out);

  fputs ("Test 1:\n", out);
  {
    int n, i;
    float x;
    char name[50];
    n = fscanf (in, "%d%f%s", &i, &x, name);
    fprintf (out, "n = %d, i = %d, x = %f, name = \"%.50s\"\n",
	     n, i, x, name);
  }
  fprintf (out, "Residual: \"%s\"\n", fgets (buf, sizeof (buf), in));
  fputs ("Test 2:\n", out);
  {
    int i;
    float x;
    char name[50];
    (void) fscanf (in, "%2d%f%*d %[0123456789]", &i, &x, name);
    fprintf (out, "i = %d, x = %f, name = \"%.50s\"\n", i, x, name);
  }
  fprintf (out, "Residual: \"%s\"\n", fgets (buf, sizeof (buf), in));
  fputs ("Test 3:\n", out);
  {
    float quant;
    char units[21], item[21];
    while (!feof (in) && !ferror (in))
      {
	int count;
	quant = 0.0;
	units[0] = item[0] = '\0';
	count = fscanf (in, "%f%20s of %20s", &quant, units, item);
	(void) fscanf (in, "%*[^\n]");
	fprintf (out, "count = %d, quant = %f, item = %.21s, units = %.21s\n",
		 count, quant, item, units);
      }
  }
  fprintf (out, "Residual: \"%s\"\n", fgets (buf, sizeof (buf), in));

  if (out != stdout)
    pclose (out);

  exit(EXIT_SUCCESS);
}
