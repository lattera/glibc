/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>
   and Paul Janzen <pcj@primenet.com>, 1996.

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

#include <assert.h>
#if _LIBC
#include <libc-lock.h>
#else
#define __libc_lock_lock(lock)		((void) 0)
#define __libc_lock_unlock(lock)	((void) 0)
#define __libc_lock_define_initialized(CLASS,NAME)
#define weak_alias(name, aliasname) \
  extern __typeof (name) aliasname __attribute__ ((weak, alias (#name))); 
#endif
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <utmp.h>

#include "utmp-private.h"


/* The various backends we have.  */
static int setutent_unknown (int reset);
static int getutent_r_unknown (struct utmp *buffer, struct utmp **result);
static struct utmp *pututline_unknown (const struct utmp *data);
static void endutent_unknown (void);

/* Initial Jump table.  */
struct utfuncs __libc_utmp_unknown_functions =
{
  setutent_unknown,
  getutent_r_unknown,
  NULL,
  NULL,
  pututline_unknown,
  endutent_unknown,
  NULL
};

/* Currently selected backend.  */
struct utfuncs *__libc_utmp_jump_table = &__libc_utmp_unknown_functions;

/* We need to protect the opening of the file.  */
__libc_lock_define_initialized (, __libc_utmp_lock)

     
void
__setutent (void)
{
  __libc_lock_lock (__libc_utmp_lock);

  (void) (*__libc_utmp_jump_table->setutent) (1);

  __libc_lock_unlock (__libc_utmp_lock);
}
weak_alias (__setutent, setutent)


static int
setutent_unknown (int reset)
{
  /* We have to test whether it is still not decided which backend to use.  */
  assert (__libc_utmp_jump_table == &__libc_utmp_unknown_functions);

  /* See whether utmpd is running.  */
  if ((*__libc_utmp_daemon_functions.setutent) (reset))
    __libc_utmp_jump_table = &__libc_utmp_daemon_functions;
  else
    {
      /* Use the normal file, but if the current file is _PATH_UTMP or
         _PATH_WTMP and the corresponding extended file (with an extra
         'x' added to the pathname) exists, we use the extended file,
         because the original file is in a different format.  */
      if (strcmp (__libc_utmp_file_name, _PATH_UTMP) == 0
	  && __access (_PATH_UTMP "x", F_OK) == 0)
	__utmpname (_PATH_UTMP "x");
      else if (strcmp (__libc_utmp_file_name, _PATH_WTMP) == 0
	       && __access (_PATH_WTMP "x", F_OK) == 0)
	__utmpname (_PATH_WTMP "x");

      (*__libc_utmp_file_functions.setutent) (reset);
      __libc_utmp_jump_table = &__libc_utmp_file_functions;
    }
  
  return 0;
}


void
__endutent (void)
{
  __libc_lock_lock (__libc_utmp_lock);

  (*__libc_utmp_jump_table->endutent) ();

  __libc_lock_unlock (__libc_utmp_lock);
}
weak_alias (__endutent, endutent)


static void
endutent_unknown (void)
{
  /* Huh, how do we came here?  Nothing to do.  */
}


int
__getutent_r (struct utmp *buffer, struct utmp **result)
{
  int retval;

  __libc_lock_lock (__libc_utmp_lock);

  retval = (*__libc_utmp_jump_table->getutent_r) (buffer, result);

  __libc_lock_unlock (__libc_utmp_lock);

  return retval;
}
weak_alias (__getutent_r, getutent_r)


static int
getutent_r_unknown (struct utmp *buffer, struct utmp **result)
{
  /* It is not yet initialized.  */
  setutent_unknown (0);

  return (*__libc_utmp_jump_table->getutent_r) (buffer, result);
}


struct utmp *
__pututline (const struct utmp *data)
{
  struct utmp *buffer;

  __libc_lock_lock (__libc_utmp_lock);

  buffer = (*__libc_utmp_jump_table->pututline) (data);

  __libc_lock_unlock (__libc_utmp_lock);

  return buffer;
}
weak_alias (__pututline, pututline)


static struct utmp *
pututline_unknown (const struct utmp *data)
{
  /* It is not yet initialized.  */
  setutent_unknown (0);

  return (*__libc_utmp_jump_table->pututline) (data);
}
