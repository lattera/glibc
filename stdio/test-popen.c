#include <ansidecl.h>
#include <stdio.h>
#include <stdlib.h>

void
DEFUN(write_data, (stream), FILE *stream)
{
  int i;
  for (i=0; i<100; i++)
    fprintf (stream, "%d\n", i);
  if (ferror (stream))  {
    fprintf (stderr, "Output to stream failed.\n");
    exit (1);
    }
}

void
DEFUN(read_data, (stream), FILE *stream)
{
  int i, j;

  for (i=0; i<100; i++)
    {
      if (fscanf (stream, "%d\n", &j) != 1 || j != i)
	{
	  if (ferror (stream))
	    perror ("fscanf");
	  puts ("Test FAILED!");
	  exit (1);
	}
    }
}

int
DEFUN_VOID(main)
{
  FILE *output, *input;
  int wstatus, rstatus;

  output = popen ("/bin/cat >tstpopen.tmp", "w");
  if (output == NULL)
    {
      perror ("popen");
      puts ("Test FAILED!");
      exit (1);
    }
  write_data (output);
  wstatus = pclose (output);
  printf ("writing pclose returned %d\n", wstatus);
  input = popen ("/bin/cat tstpopen.tmp", "r");
  if (input == NULL)
    {
      perror ("tstpopen.tmp");
      puts ("Test FAILED!");
      exit (1);
    }
  read_data (input);
  rstatus = pclose (input);
  printf ("reading pclose returned %d\n", rstatus);

  puts (wstatus | rstatus  ? "Test FAILED!" : "Test succeeded.");
  exit (wstatus | rstatus);
}

  

  
