/* Prepare arguments for library initialization function.
   Copyright (C) 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* The job of this fragment it to find argc and friends for INIT.
   This is done in one of two ways: either in the stack context
   of program start, or having dlopen pass them in.  */

#define SYSDEP_CALL_INIT(NAME, INIT)					      \
void NAME (void *arg)							      \
{									      \
  int argc;								      \
  char **argv, **envp;							      \
  /* The next variable is only here to work around a bug in gcc <= 2.7.2.2.   \
     If the address would be taken inside the expression the optimizer	      \
     would try to be too smart and throws it away.  Grrr.  */		      \
  int *dummy_addr = &_dl_starting_up;					      \
									      \
  __libc_multiple_libcs = dummy_addr && !_dl_starting_up;		      \
									      \
  if (!__libc_multiple_libcs)						      \
    {									      \
      argc = *(int *) arg;						      \
      argv = (char **) (arg + 4);					      \
      envp = &argv[argc+1];						      \
    }									      \
  else									      \
    {									      \
      argc = (int) arg;							      \
      argv = ((char ***) &arg)[1];					      \
      envp = ((char ***) &arg)[2];					      \
    }									      \
									      \
  INIT (argc, argv, envp);						      \
}


