/* Copyright (C) 1996-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>
   and Paul Janzen <pcj@primenet.com>, 1996.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <libc-lock.h>
#include <stdlib.h>
#include <utmp.h>

#include "utmp-private.h"


/* Functions defined here.  */
static int setutent_unknown (void);
static int getutent_r_unknown (struct utmp *buffer, struct utmp **result);
static int getutid_r_unknown (const struct utmp *line, struct utmp *buffer,
			      struct utmp **result);
static int getutline_r_unknown (const struct utmp *id, struct utmp *buffer,
				struct utmp **result);
static struct utmp *pututline_unknown (const struct utmp *data);
static void endutent_unknown (void);

/* Initial Jump table.  */
const struct utfuncs __libc_utmp_unknown_functions =
{
  setutent_unknown,
  getutent_r_unknown,
  getutid_r_unknown,
  getutline_r_unknown,
  pututline_unknown,
  endutent_unknown,
  NULL
};

/* Currently selected backend.  */
const struct utfuncs *__libc_utmp_jump_table = &__libc_utmp_unknown_functions;

/* We need to protect the opening of the file.  */
__libc_lock_define_initialized (, __libc_utmp_lock attribute_hidden)


static int
setutent_unknown (void)
{
  int result;

  result = (*__libc_utmp_file_functions.setutent) ();
  if (result)
    __libc_utmp_jump_table = &__libc_utmp_file_functions;

  return result;
}


static int
getutent_r_unknown (struct utmp *buffer, struct utmp **result)
{
  /* The backend was not yet initialized.  */
  if (setutent_unknown ())
    return (*__libc_utmp_jump_table->getutent_r) (buffer, result);

  /* Not available.  */
  *result = NULL;
  return -1;
}


static int
getutid_r_unknown (const struct utmp *id, struct utmp *buffer,
		   struct utmp **result)
{
  /* The backend was not yet initialized.  */
  if (setutent_unknown ())
    return (*__libc_utmp_jump_table->getutid_r) (id, buffer, result);

  /* Not available.  */
  *result = NULL;
  return -1;
}


static int
getutline_r_unknown (const struct utmp *line, struct utmp *buffer,
		     struct utmp **result)
{
  /* The backend was not yet initialized.  */
  if (setutent_unknown ())
    return (*__libc_utmp_jump_table->getutline_r) (line, buffer, result);

  /* Not available.  */
  *result = NULL;
  return -1;
}


static struct utmp *
pututline_unknown (const struct utmp *data)
{
  /* The backend was not yet initialized.  */
  if (setutent_unknown ())
    return (*__libc_utmp_jump_table->pututline) (data);

  /* Not available.  */
  return NULL;
}


static void
endutent_unknown (void)
{
  /* Nothing to do.  */
}


void
__setutent (void)
{
  __libc_lock_lock (__libc_utmp_lock);

  (*__libc_utmp_jump_table->setutent) ();

  __libc_lock_unlock (__libc_utmp_lock);
}
weak_alias (__setutent, setutent)


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


void
__endutent (void)
{
  __libc_lock_lock (__libc_utmp_lock);

  (*__libc_utmp_jump_table->endutent) ();
  __libc_utmp_jump_table = &__libc_utmp_unknown_functions;

  __libc_lock_unlock (__libc_utmp_lock);
}
weak_alias (__endutent, endutent)
