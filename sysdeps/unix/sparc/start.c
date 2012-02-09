/* Copyright (C) 1991-1997,2004,2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Lesser General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file with other
   programs, and to distribute those programs without any restriction
   coming from the use of this file. (The GNU Lesser General Public
   License restrictions do apply in other respects; for example, they
   cover modification of the file, and distribution when not linked
   into another program.)

   Note that people who make modified versions of this file are not
   obligated to grant this special exception for their modified
   versions; it is their choice whether to do so. The GNU Lesser
   General Public License gives permission to release a modified
   version without this exception; this exception also makes it
   possible to release a modified version which carries forward this
   exception.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef NO_SHLIB
#include <sys/exec.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <link.h>
#include <syscall.h>
#endif

#if !defined (__GNUC__) || __GNUC__ < 2
  #error This file uses GNU C extensions; you must compile with GCC version 2.
#endif

/* The first piece of initialized data.  */
int __data_start = 0;
weak_alias (__data_start, data_start)

extern void __libc_init (int argc, char **argv, char **envp) __THROW;
extern int main (int argc, char **argv, char **envp) __THROW;

register long int sp asm("%sp"), fp asm("%fp");

#ifndef NO_SHLIB
static void init_shlib (void) __THROW;
#endif

#ifndef NO_EXPLICIT_START
/* Declare _start with an explicit assembly symbol name of `start'
   (note no leading underscore).  This is the name Sun's crt0.o uses,
   and programs are often linked with `ld -e start'.  */
void _start (void) asm ("start");
#endif

void
_start (void)
{
  /* It is important that these be declared `register'.
     Otherwise, when compiled without optimization, they are put on the
     stack, which loses completely after we zero the FP.  */
  register int argc;
  register char **argv, **envp;

  /* Unwind the frame built when we entered the function.  */
  asm("restore");

  /* And clear the frame pointer.  */
  fp = 0;

  /* The argument info starts after one register
     window (64 bytes) past the SP.  */
  argc = ((int *) sp)[16];
  argv = (char **) &((int *) sp)[17];
  envp = &argv[argc + 1];
  __environ = envp;

#ifndef NO_SHLIB
  init_shlib ();
#endif

  /* Allocate 24 bytes of stack space for the register save area.  */
  sp -= 24;
  __libc_init (argc, argv, envp);

  exit (main (argc, argv, envp));
}

#ifndef NO_SHLIB

/* System calls for use by the bootstrap routine.
   These are defined here since the usual calls may be dynamically linked.  */

int syscall (int sysno, ...) asm ("init_syscall");
asm ("init_syscall:\n"
     "	clr %g1\n"
     "	ta 0\n"
     "	bcc 1f\n"
     "	sethi %hi(_errno), %g1\n"
     "	st %o0, [%g1 + %lo(_errno)]\n"
     "	sub %g0, 1, %o0\n"
     "1:retl\n"
     "	nop");

static void
init_shlib ()
{
  extern struct link_dynamic _DYNAMIC;
  int so, zf;
  caddr_t somap;
  caddr_t sodmap;
  caddr_t sobssmap;
  void (*ldstart) (int, int);
  struct exec soexec;
  struct
    {
      caddr_t crt_ba;
      int crt_dzfd;
      int crt_ldfd;
      struct link_dynamic *crt_dp;
      char **crt_ep;
      caddr_t crt_bp;
    } soarg;

  /* If not dynamically linked, do nothing.  */
  if (&_DYNAMIC == 0)
    return;

  /* Map in the dynamic linker.  */
  so = syscall (SYS_open, "/usr/lib/ld.so", O_RDONLY);
  if (syscall (SYS_read, so, &soexec, sizeof (soexec)) != sizeof (soexec)
      || soexec.a_magic != ZMAGIC)
    {
      static const char emsg[] = "crt0: no /usr/lib/ld.so\n";

      syscall (SYS_write, 2, emsg, sizeof (emsg) - 1);
      syscall (SYS_exit, 127);
    }
  somap = (caddr_t) syscall (SYS_mmap, 0,
			     soexec.a_text + soexec.a_data + soexec.a_bss,
			     PROT_READ | PROT_EXEC, _MAP_NEW | MAP_PRIVATE,
			     so, 0);
  sodmap = (caddr_t) syscall (SYS_mmap, somap + soexec.a_text, soexec.a_data,
			      PROT_READ | PROT_WRITE | PROT_EXEC,
			      _MAP_NEW | MAP_FIXED | MAP_PRIVATE,
			      so, soexec.a_text);
  zf = syscall (SYS_open, "/dev/zero", O_RDONLY);
  if (soexec.a_bss != 0)
    sobssmap = (caddr_t) syscall (SYS_mmap,
				  somap + soexec.a_text + soexec.a_data,
				  soexec.a_bss,
				  PROT_READ | PROT_WRITE | PROT_EXEC,
				  _MAP_NEW | MAP_FIXED | MAP_PRIVATE,
				  zf, 0);

  /* Call the entry point of the dynamic linker.  */
  soarg.crt_ba = somap;
  soarg.crt_dzfd = zf;
  soarg.crt_ldfd = so;
  soarg.crt_dp = &_DYNAMIC;
  soarg.crt_ep = __environ;
  soarg.crt_bp = (caddr_t) &&retaddr;

  ldstart = (__typeof (ldstart)) (somap + soexec.a_entry);
  (*ldstart) (1, (char *) &soarg - (char *) sp);

 retaddr:
}

#endif
