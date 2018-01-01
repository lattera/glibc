/* Template generic NSS service provider.  See nss_test.h for usage.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <errno.h>
#include <nss.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <alloc_buffer.h>


/* We need to be able to handle NULLs "properly" within the testsuite,
   to test known bad data.  */
#define alloc_buffer_maybe_copy_string(b,s) s ? alloc_buffer_copy_string (b, s) : NULL;

/* This file is the master template.  Other instances of this test
   module should define NAME(x) to have their name instead of "test1",
   then include this file.
*/
#define NAME_(x,n) _nss_##n##_##x
#ifndef NAME
#define NAME(x) NAME_(x,test1)
#endif
#define NAMESTR__(x) #x
#define NAMESTR_(x) NAMESTR__(x)
#define NAMESTR(x) NAMESTR_(NAME(x))

#include "nss_test.h"

/* -------------------------------------------------- */
/* Default Data.  */

static struct passwd default_pwd_data[] =
  {
#define PWD(u) \
    { .pw_name = (char *) "name" #u, .pw_passwd = (char *) "*", .pw_uid = u,  \
      .pw_gid = 100, .pw_gecos = (char *) "*", .pw_dir = (char *) "*",	      \
      .pw_shell = (char *) "*" }
    PWD (30),
    PWD (100),
    PWD (200),
    PWD (60),
    PWD (20000)
  };
#define default_npwd_data \
  (sizeof (default_pwd_data) / sizeof (default_pwd_data[0]))

static struct passwd *pwd_data = default_pwd_data;
static int npwd_data = default_npwd_data;

static struct group *grp_data = NULL;
static int ngrp_data = 0;

/* This function will get called, and once per session, look back into
   the test case's executable for an init hook function, and call
   it.  */

static int initted = 0;
static void
init(void)
{
  test_tables t;
  int i;

  if (initted)
    return;
  if (NAME(init_hook))
    {
      memset (&t, 0, sizeof(t));
      NAME(init_hook)(&t);

      if (t.pwd_table)
	{
	  pwd_data = t.pwd_table;
	  for (i=0; ! PWD_ISLAST(& pwd_data[i]); i++)
	    ;
	  npwd_data = i;
	}

      if (t.grp_table)
	{
	  grp_data = t.grp_table;
	  for (i=0; ! GRP_ISLAST(& grp_data[i]); i++)
	    ;
	  ngrp_data = i;
	}
    }
  initted = 1;
}

/* -------------------------------------------------- */
/* Password handling.  */

static size_t pwd_iter;
#define CURPWD pwd_data[pwd_iter]

static pthread_mutex_t pwd_lock = PTHREAD_MUTEX_INITIALIZER;

enum nss_status
NAME(setpwent) (int stayopen)
{
  init();
  pwd_iter = 0;
  return NSS_STATUS_SUCCESS;
}


enum nss_status
NAME(endpwent) (void)
{
  init();
  return NSS_STATUS_SUCCESS;
}

static enum nss_status
copy_passwd (struct passwd *result, struct passwd *local,
	    char *buffer, size_t buflen, int *errnop)
{
  struct alloc_buffer buf = alloc_buffer_create (buffer, buflen);

  result->pw_name = alloc_buffer_maybe_copy_string (&buf, local->pw_name);
  result->pw_passwd = alloc_buffer_maybe_copy_string (&buf, local->pw_passwd);
  result->pw_uid = local->pw_uid;
  result->pw_gid = local->pw_gid;
  result->pw_gecos = alloc_buffer_maybe_copy_string (&buf, local->pw_gecos);
  result->pw_dir = alloc_buffer_maybe_copy_string (&buf, local->pw_dir);
  result->pw_shell = alloc_buffer_maybe_copy_string (&buf, local->pw_shell);

  if (alloc_buffer_has_failed (&buf))
    {
      *errnop = ERANGE;
      return NSS_STATUS_TRYAGAIN;
    }

  return NSS_STATUS_SUCCESS;
}

enum nss_status
NAME(getpwent_r) (struct passwd *result, char *buffer, size_t buflen,
		       int *errnop)
{
  int res = NSS_STATUS_SUCCESS;

  init();
  pthread_mutex_lock (&pwd_lock);

  if (pwd_iter >= npwd_data)
    res = NSS_STATUS_NOTFOUND;
  else
    {
      res = copy_passwd (result, &CURPWD, buffer, buflen, errnop);
      ++pwd_iter;
    }

  pthread_mutex_unlock (&pwd_lock);

  return res;
}


enum nss_status
NAME(getpwuid_r) (uid_t uid, struct passwd *result, char *buffer,
		       size_t buflen, int *errnop)
{
  init();
  for (size_t idx = 0; idx < npwd_data; ++idx)
    if (pwd_data[idx].pw_uid == uid)
      return copy_passwd (result, &pwd_data[idx], buffer, buflen, errnop);

  return NSS_STATUS_NOTFOUND;
}


enum nss_status
NAME(getpwnam_r) (const char *name, struct passwd *result, char *buffer,
		       size_t buflen, int *errnop)
{
  init();
  for (size_t idx = 0; idx < npwd_data; ++idx)
    if (strcmp (pwd_data[idx].pw_name, name) == 0)
      return copy_passwd (result, &pwd_data[idx], buffer, buflen, errnop);

  return NSS_STATUS_NOTFOUND;
}

/* -------------------------------------------------- */
/* Group handling.  */

static size_t grp_iter;
#define CURGRP grp_data[grp_iter]

static pthread_mutex_t grp_lock = PTHREAD_MUTEX_INITIALIZER;

enum nss_status
NAME(setgrent) (int stayopen)
{
  init();
  grp_iter = 0;
  return NSS_STATUS_SUCCESS;
}


enum nss_status
NAME(endgrent) (void)
{
  init();
  return NSS_STATUS_SUCCESS;
}

static enum nss_status
copy_group (struct group *result, struct group *local,
	    char *buffer, size_t buflen, int *errnop)
{
  struct alloc_buffer buf = alloc_buffer_create (buffer, buflen);
  char **memlist;
  int i;

  if (local->gr_mem)
    {
      i = 0;
      while (local->gr_mem[i])
	++i;

      memlist = alloc_buffer_alloc_array (&buf, char *, i + 1);

      if (memlist) {
	for (i = 0; local->gr_mem[i]; ++i)
	  memlist[i] = alloc_buffer_maybe_copy_string (&buf, local->gr_mem[i]);
	memlist[i] = NULL;
      }

      result->gr_mem = memlist;
    }
  else
    result->gr_mem = NULL;

  result->gr_name = alloc_buffer_maybe_copy_string (&buf, local->gr_name);
  result->gr_passwd = alloc_buffer_maybe_copy_string (&buf, local->gr_passwd);
  result->gr_gid = local->gr_gid;

  if (alloc_buffer_has_failed (&buf))
    {
      *errnop = ERANGE;
      return NSS_STATUS_TRYAGAIN;
    }

  return NSS_STATUS_SUCCESS;
}


enum nss_status
NAME(getgrent_r) (struct group *result, char *buffer, size_t buflen,
		       int *errnop)
{
  int res = NSS_STATUS_SUCCESS;

  init();
  pthread_mutex_lock (&grp_lock);

  if (grp_iter >= ngrp_data)
    res = NSS_STATUS_NOTFOUND;
  else
    {
      res = copy_group (result, &CURGRP, buffer, buflen, errnop);
      ++grp_iter;
    }

  pthread_mutex_unlock (&pwd_lock);

  return res;
}


enum nss_status
NAME(getgrgid_r) (gid_t gid, struct group *result, char *buffer,
		  size_t buflen, int *errnop)
{
  init();
  for (size_t idx = 0; idx < ngrp_data; ++idx)
    if (grp_data[idx].gr_gid == gid)
      return copy_group (result, &grp_data[idx], buffer, buflen, errnop);

  return NSS_STATUS_NOTFOUND;
}


enum nss_status
NAME(getgrnam_r) (const char *name, struct group *result, char *buffer,
		       size_t buflen, int *errnop)
{
  init();
  for (size_t idx = 0; idx < ngrp_data; ++idx)
    if (strcmp (pwd_data[idx].pw_name, name) == 0)
      {
	return copy_group (result, &grp_data[idx], buffer, buflen, errnop);
      }

  return NSS_STATUS_NOTFOUND;
}
