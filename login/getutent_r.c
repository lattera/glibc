/* Copyright (C) 1996 Free Software Foundation, Inc.
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
#include <db.h>
#include <fcntl.h>
#include <libc-lock.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utmp.h>
#include <gnu/lib-names.h>
#include <sys/stat.h>

#include "utmp-private.h"
#include "../elf/link.h"


/* The various backends we have.  */
static int __setutent_unknown (int reset);
static int __getutent_r_unknown (struct utmp *buffer, struct utmp **result);
static void __pututline_unknown (const struct utmp *data);
static void __endutent_unknown (void);


/* We have three jump tables: unknown, db, or file.  */
static struct utfuncs unknown_functions =
{
  __setutent_unknown,
  __getutent_r_unknown,
  NULL,
  NULL,
  __pututline_unknown,
  __endutent_unknown,
  NULL
};

/* Currently selected backend.  */
struct utfuncs *__libc_utmp_jump_table = &unknown_functions;

/* The tables from the services.  */
extern struct utfuncs __libc_utmp_db_functions;
extern struct utfuncs __libc_utmp_file_functions;


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
__setutent_unknown (int reset)
{
  /* We have to test whether it is still not decided which backend to use.  */
  assert (__libc_utmp_jump_table == &unknown_functions);

  /* See whether utmp db file exists.  */
  if ((*__libc_utmp_db_functions.setutent) (reset))
    __libc_utmp_jump_table = &__libc_utmp_db_functions;
  else
    {
      /* Either the db file does not exist or we have other
	 problems.  So use the normal file.  */
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
__endutent_unknown (void)
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
__getutent_r_unknown (struct utmp *buffer, struct utmp **result)
{
  /* It is not yet initialized.  */
  __setutent_unknown (0);

  return (*__libc_utmp_jump_table->getutent_r) (buffer, result);
}


void
__pututline (const struct utmp *data)
{
  __libc_lock_lock (__libc_utmp_lock);

  (*__libc_utmp_jump_table->pututline) (data);

  __libc_lock_unlock (__libc_utmp_lock);
}


static void
__pututline_unknown (const struct utmp *data)
{
  /* It is not yet initialized.  */
  __setutent_unknown (0);

  (*__libc_utmp_jump_table->pututline) (data);
}


int
__utmpname (const char *file)
{
  int result = -1;

  __libc_lock_lock (__libc_utmp_lock);

  /* Close the old file.  */
  (*__libc_utmp_jump_table->endutent) ();

  /* Store new names.  */
  if ((*__libc_utmp_file_functions.utmpname) (file) == 0
      && !(*__libc_utmp_db_functions.utmpname) (file) == 0)
    {
      /* Try to find out whether we are supposed to work with a db
	 file or not.  Do this by looking for the extension ".db".  */
      const char *ext = strrchr (file, '.');

      if (ext != NULL && strcmp (ext, ".db") == 0)
	__libc_utmp_jump_table = &__libc_utmp_db_functions;
      else
	__libc_utmp_jump_table = &unknown_functions;

      result = 0;
    }

  __libc_lock_unlock (__libc_utmp_lock);

  return result;
}
weak_alias (__utmpname, utmpname)
