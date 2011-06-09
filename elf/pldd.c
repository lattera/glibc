/* List dynamic shared objects linked into given process.
   Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gmail.com>, 2011.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <alloca.h>
#include <argp.h>
#include <elf.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <libintl.h>
#include <link.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/stat.h>

#include <ldsodefs.h>
#include <version.h>

/* Global variables.  */
extern char *program_invocation_short_name;
#define PACKAGE _libc_intl_domainname

/* External functions.  */
extern void *xmalloc (size_t n);
extern void *xrealloc (void *p, size_t n);

/* Name and version of program.  */
static void print_version (FILE *stream, struct argp_state *state);
void (*argp_program_version_hook) (FILE *, struct argp_state *) = print_version;

/* Bug report address.  */
const char *argp_program_bug_address = N_("\
For bug reporting instructions, please see:\n\
<http://www.gnu.org/software/libc/bugs.html>.\n");

/* Definitions of arguments for argp functions.  */
static const struct argp_option options[] =
{
  { NULL, 0, NULL, 0, NULL }
};

/* Short description of program.  */
static const char doc[] = N_("\
List dynamic shared objects loaded into process.");

/* Strings for arguments in help texts.  */
static const char args_doc[] = N_("PID");

/* Prototype for option handler.  */
static error_t parse_opt (int key, char *arg, struct argp_state *state);

/* Data structure to communicate with argp functions.  */
static struct argp argp =
{
  options, parse_opt, args_doc, doc, NULL, NULL, NULL
};

// File descriptor of /proc/*/mem file.
static int memfd;

/* Name of the executable  */
static char *exe;

/* Local functions.  */
static int get_process_info (pid_t pid);


int
main (int argc, char *argv[])
{
  /* Parse and process arguments.  */
  int remaining;
  argp_parse (&argp, argc, argv, 0, &remaining, NULL);

  if (remaining != argc - 1)
    {
      fprintf (stderr,
	       gettext ("Exactly one parameter with process ID required.\n"));
      argp_help (&argp, stderr, ARGP_HELP_SEE, program_invocation_short_name);
      return 1;
    }

  char *endp;
  errno = 0;
  pid_t pid = strtoul (argv[remaining], &endp, 10);
  if ((pid == ULONG_MAX && errno == ERANGE) || *endp != '\0')
    error (EXIT_FAILURE, 0, gettext ("invalid process ID '%s'"),
	   argv[remaining]);

  /* Determine the program name.  */
  char buf[11 + 3 * sizeof (pid)];
  snprintf (buf, sizeof (buf), "/proc/%lu/exe", (unsigned long int) pid);
  size_t exesize = 1024;
  exe = alloca (exesize);
  ssize_t nexe;
  while ((nexe = readlink (buf, exe, exesize)) == exesize)
    extend_alloca (exe, exesize, 2 * exesize);
  if (nexe == -1)
    exe = (char *) "<program name undetermined>";
  else
    exe[nexe] = '\0';

  if (ptrace (PTRACE_ATTACH, pid, NULL, NULL) != 0)
    error (EXIT_FAILURE, errno, gettext ("cannot attach to process %lu"),
	   (unsigned long int) pid);

  int status = get_process_info (pid);

  ptrace (PTRACE_DETACH, pid, NULL, NULL);

  return status;
}


/* Handle program arguments.  */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  switch (key)
    {
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}


/* Print the version information.  */
static void
print_version (FILE *stream, struct argp_state *state)
{
  fprintf (stream, "pldd (GNU %s) %s\n", PACKAGE, VERSION);
  fprintf (stream, gettext ("\
Copyright (C) %s Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
"), "2011");
  fprintf (stream, gettext ("Written by %s.\n"), "Ulrich Drepper");
}


#define CLASS 32
#include "pldd-xx.c"
#define CLASS 64
#include "pldd-xx.c"


static int
get_process_info (pid_t pid)
{
  char buf[12 + 3 * sizeof (pid)];

  snprintf (buf, sizeof (buf), "/proc/%lu/mem", (unsigned long int) pid);
  memfd = open (buf, O_RDONLY);
  if (memfd == -1)
    goto no_info;

  snprintf (buf, sizeof (buf), "/proc/%lu/exe", (unsigned long int) pid);
  int fd = open (buf, O_RDONLY);
  if (fd == -1)
    {
    no_info:
      error (0, errno, gettext ("cannot get information about process %lu"),
	     (unsigned long int) pid);
      return EXIT_FAILURE;
    }

  union
  {
    Elf32_Ehdr ehdr32;
    Elf64_Ehdr ehdr64;
  } uehdr;
  if (read (fd, &uehdr, sizeof (uehdr)) != sizeof (uehdr))
    goto no_info;

  close (fd);

  if (memcmp (uehdr.ehdr32.e_ident, ELFMAG, SELFMAG) != 0)
    {
      error (0, 0, gettext ("process %lu is no ELF program"),
	     (unsigned long int) pid);
      return EXIT_FAILURE;
    }

  snprintf (buf, sizeof (buf), "/proc/%lu/auxv", (unsigned long int) pid);
  fd = open (buf, O_RDONLY);
  if (fd == -1)
    goto no_info;

  size_t auxv_size = 0;
  void *auxv = NULL;
  while (1)
    {
      auxv_size += 512;
      auxv = xrealloc (auxv, auxv_size);

      ssize_t n = read (fd, auxv, auxv_size);
      if (n < 0)
	goto no_info;
      if (n < auxv_size)
	{
	  auxv_size = n;
	  break;
	}
    }

  close (fd);

  if (uehdr.ehdr32.e_ident[EI_CLASS] == ELFCLASS32)
    return find_maps32 (pid, &uehdr.ehdr32, auxv, auxv_size);
  else
    return find_maps64 (pid, &uehdr.ehdr64, auxv, auxv_size);
}
