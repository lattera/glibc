/* Copyright (C) 1992,93,94,95,96,97,99,2001 Free Software Foundation, Inc.
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

#include <hurd.h>
#include <hurd/lookup.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include "stdio-common/_itoa.h"
#include <hurd/term.h>
#include <hurd/paths.h>


/* Translate the error from dir_lookup into the error the user sees.  */
static inline error_t
lookup_error (error_t error)
{
  switch (error)
    {
    case EOPNOTSUPP:
    case MIG_BAD_ID:
      /* These indicate that the server does not understand dir_lookup
	 at all.  If it were a directory, it would, by definition.  */
      return ENOTDIR;
    default:
      return error;
    }
}

error_t
__hurd_file_name_lookup (error_t (*use_init_port)
			   (int which, error_t (*operate) (file_t)),
			 file_t (*get_dtable_port) (int fd),
			 error_t (*lookup)
			   (file_t dir, char *name, int flags, mode_t mode,
			    retry_type *do_retry, string_t retry_name,
			    mach_port_t *result),
			 const char *file_name, int flags, mode_t mode,
			 file_t *result)
{
  error_t err;
  enum retry_type doretry;
  char retryname[1024];		/* XXX string_t LOSES! */
  int startport;

  error_t lookup_op (mach_port_t startdir)
    {
      return lookup_error ((*lookup) (startdir, file_name, flags, mode,
				      &doretry, retryname, result));
    }

  if (! lookup)
    lookup = __dir_lookup;

  startport = (file_name[0] == '/') ? INIT_PORT_CRDIR : INIT_PORT_CWDIR;
  while (file_name[0] == '/')
    file_name++;

  if (flags & O_NOFOLLOW)	/* See comments below about O_NOFOLLOW.  */
    flags |= O_NOTRANS;

  if (flags & O_DIRECTORY)
    {
      /* The caller wants to require that the file we look up is a directory.
	 We can do this without an extra RPC by appending a trailing slash
	 to the file name we look up.  */
      size_t len = strlen (file_name);
      if (len == 0)
	file_name = "/";
      else if (file_name[len - 1] != '/')
	{
	  char *n = alloca (len + 2);
	  memcpy (n, file_name, len);
	  n[len] = '/';
	  n[len + 1] = '\0';
	  file_name = n;
	}
    }

  err = (*use_init_port) (startport, &lookup_op);
  if (! err)
    err = __hurd_file_name_lookup_retry (use_init_port, get_dtable_port,
					 lookup, doretry, retryname,
					 flags, mode, result);

  return err;
}
weak_alias (__hurd_file_name_lookup, hurd_file_name_lookup)

error_t
__hurd_file_name_lookup_retry (error_t (*use_init_port)
			         (int which, error_t (*operate) (file_t)),
			       file_t (*get_dtable_port) (int fd),
			       error_t (*lookup)
			         (file_t dir, char *name,
				  int flags, mode_t mode,
				  retry_type *do_retry, string_t retry_name,
				  mach_port_t *result),
			       enum retry_type doretry,
			       char retryname[1024],
			       int flags, mode_t mode,
			       file_t *result)
{
  error_t err;
  char *file_name;
  int nloops;

  error_t lookup_op (file_t startdir)
    {
      while (file_name[0] == '/')
	file_name++;

      return lookup_error ((*lookup) (startdir, file_name, flags, mode,
				      &doretry, retryname, result));
    }
  error_t reauthenticate (file_t unauth)
    {
      error_t err;
      mach_port_t ref = __mach_reply_port ();
      error_t reauth (auth_t auth)
	{
	  return __auth_user_authenticate (auth, ref,
					   MACH_MSG_TYPE_MAKE_SEND,
					   result);
	}
      err = __io_reauthenticate (unauth, ref, MACH_MSG_TYPE_MAKE_SEND);
      if (! err)
	err = (*use_init_port) (INIT_PORT_AUTH, &reauth);
      __mach_port_destroy (__mach_task_self (), ref);
      __mach_port_deallocate (__mach_task_self (), unauth);
      return err;
    }

  if (! lookup)
    lookup = __dir_lookup;

  nloops = 0;
  err = 0;
  do
    {
      file_t startdir = MACH_PORT_NULL;
      int dirport = INIT_PORT_CWDIR;

      switch (doretry)
	{
	case FS_RETRY_REAUTH:
	  if (err = reauthenticate (*result))
	    return err;
	  /* Fall through.  */

	case FS_RETRY_NORMAL:
	  if (nloops++ >= SYMLOOP_MAX)
	    {
	      __mach_port_deallocate (__mach_task_self (), *result);
	      return ELOOP;
	    }

	  /* An empty RETRYNAME indicates we have the final port.  */
	  if (retryname[0] == '\0' &&
	      /* If reauth'd, we must do one more retry on "" to give the new
		 translator a chance to make a new port for us.  */
	      doretry == FS_RETRY_NORMAL)
	    {
	      if (flags & O_NOFOLLOW)
		{
		  /* In Linux, O_NOFOLLOW means to reject symlinks.  If we
		     did an O_NOLINK lookup above and io_stat here to check
		     for S_IFLNK, a translator like firmlink could easily
		     spoof this check by not showing S_IFLNK, but in fact
		     redirecting the lookup to some other name
		     (i.e. opening the very same holes a symlink would).

		     Instead we do an O_NOTRANS lookup above, and stat the
		     underlying node: if it has a translator set, and its
		     owner is not root (st_uid 0) then we reject it.
		     Since the motivation for this feature is security, and
		     that security presumes we trust the containing
		     directory, this check approximates the security of
		     refusing symlinks while accepting mount points.
		     Note that we actually permit something Linux doesn't:
		     we follow root-owned symlinks; if that is deemed
		     undesireable, we can add a final check for that
		     one exception to our general translator-based rule.  */
		  struct stat st;
		  err = __io_stat (*result, &st);
		  if (!err
		      && (st.st_mode & (S_IPTRANS|S_IATRANS)))
		    {
		      if (st.st_uid != 0)
			err = ENOENT;
		      else if (st.st_mode & S_IPTRANS)
			{
			  char buf[1024];
			  char *trans = buf;
			  size_t translen = sizeof buf;
			  err = __file_get_translator (*result,
						       &trans, &translen);
			  if (!err
			      && translen > sizeof _HURD_SYMLINK
			      && !memcmp (trans,
					  _HURD_SYMLINK, sizeof _HURD_SYMLINK))
			    err = ENOENT;
			}
		    }
		}

	      /* We got a successful translation.  Now apply any open-time
		 action flags we were passed.  */

	      if (!err && (flags & O_TRUNC)) /* Asked to truncate the file.  */
		err = __file_set_size (*result, 0);

	      if (err)
		__mach_port_deallocate (__mach_task_self (), *result);
	      return err;
	    }

	  startdir = *result;
	  file_name = retryname;
	  break;

	case FS_RETRY_MAGICAL:
	  switch (retryname[0])
	    {
	    case '/':
	      dirport = INIT_PORT_CRDIR;
	      if (*result != MACH_PORT_NULL)
		__mach_port_deallocate (__mach_task_self (), *result);
	      if (nloops++ >= SYMLOOP_MAX)
		return ELOOP;
	      file_name = &retryname[1];
	      break;

	    case 'f':
	      if (retryname[1] == 'd' && retryname[2] == '/')
		{
		  int fd;
		  char *end;
		  int save = errno;
		  errno = 0;
		  fd = (int) strtol (&retryname[3], &end, 10);
		  if (end == NULL || errno || /* Malformed number.  */
		      /* Check for excess text after the number.  A slash
			 is valid; it ends the component.  Anything else
			 does not name a numeric file descriptor.  */
		      (*end != '/' && *end != '\0'))
		    {
		      errno = save;
		      return ENOENT;
		    }
		  if (! get_dtable_port)
		    err = EGRATUITOUS;
		  else
		    {
		      *result = (*get_dtable_port) (fd);
		      if (*result == MACH_PORT_NULL)
			{
			  /* If the name was a proper number, but the file
			     descriptor does not exist, we return EBADF instead
			     of ENOENT.  */
			  err = errno;
			  errno = save;
			}
		    }
		  errno = save;
		  if (err)
		    return err;
		  if (*end == '\0')
		    return 0;
		  else
		    {
		      /* Do a normal retry on the remaining components.  */
		      startdir = *result;
		      file_name = end + 1; /* Skip the slash.  */
		      break;
		    }
		}
	      else
		goto bad_magic;
	      break;

	    case 'm':
	      if (retryname[1] == 'a' && retryname[2] == 'c' &&
		  retryname[3] == 'h' && retryname[4] == 't' &&
		  retryname[5] == 'y' && retryname[6] == 'p' &&
		  retryname[7] == 'e')
		{
		  error_t err;
		  struct host_basic_info hostinfo;
		  mach_msg_type_number_t hostinfocnt = HOST_BASIC_INFO_COUNT;
		  char *p;
		  /* XXX want client's host */
		  if (err = __host_info (__mach_host_self (), HOST_BASIC_INFO,
					 (natural_t *) &hostinfo,
					 &hostinfocnt))
		    return err;
		  if (hostinfocnt != HOST_BASIC_INFO_COUNT)
		    return EGRATUITOUS;
		  p = _itoa (hostinfo.cpu_subtype, &retryname[8], 10, 0);
		  *--p = '/';
		  p = _itoa (hostinfo.cpu_type, &retryname[8], 10, 0);
		  if (p < retryname)
		    abort ();	/* XXX write this right if this ever happens */
		  if (p > retryname)
		    strcpy (retryname, p);
		  startdir = *result;
		}
	      else
		goto bad_magic;
	      break;

	    case 't':
	      if (retryname[1] == 't' && retryname[2] == 'y')
		switch (retryname[3])
		  {
		    error_t opentty (file_t *result)
		      {
			error_t err;
			error_t ctty_open (file_t port)
			  {
			    if (port == MACH_PORT_NULL)
			      return ENXIO; /* No controlling terminal.  */
			    return __termctty_open_terminal (port,
							     flags,
							     result);
			  }
			err = (*use_init_port) (INIT_PORT_CTTYID, &ctty_open);
			if (! err)
			  err = reauthenticate (*result);
			return err;
		      }

		  case '\0':
		    return opentty (result);
		  case '/':
		    if (err = opentty (&startdir))
		      return err;
		    strcpy (retryname, &retryname[4]);
		    break;
		  default:
		    goto bad_magic;
		  }
	      else
		goto bad_magic;
	      break;

	    default:
	    bad_magic:
	      return EGRATUITOUS;
	    }
	  break;

	default:
	  return EGRATUITOUS;
	}

      if (startdir != MACH_PORT_NULL)
	{
	  err = lookup_op (startdir);
	  __mach_port_deallocate (__mach_task_self (), startdir);
	  startdir = MACH_PORT_NULL;
	}
      else
	err = (*use_init_port) (dirport, &lookup_op);
    } while (! err);

  return err;
}
weak_alias (__hurd_file_name_lookup_retry, hurd_file_name_lookup_retry)

error_t
__hurd_file_name_split (error_t (*use_init_port)
			  (int which, error_t (*operate) (file_t)),
			file_t (*get_dtable_port) (int fd),
			error_t (*lookup)
			  (file_t dir, char *name, int flags, mode_t mode,
			   retry_type *do_retry, string_t retry_name,
			   mach_port_t *result),
			const char *file_name,
			file_t *dir, char **name)
{
  error_t addref (file_t crdir)
    {
      *dir = crdir;
      return __mach_port_mod_refs (__mach_task_self (),
				   crdir, MACH_PORT_RIGHT_SEND, +1);
    }

  const char *lastslash = strrchr (file_name, '/');

  if (lastslash != NULL)
    {
      if (lastslash == file_name)
	{
	  /* "/foobar" => crdir + "foobar".  */
	  *name = (char *) file_name + 1;
	  return (*use_init_port) (INIT_PORT_CRDIR, &addref);
	}
      else
	{
	  /* "/dir1/dir2/.../file".  */
	  char dirname[lastslash - file_name + 1];
	  memcpy (dirname, file_name, lastslash - file_name);
	  dirname[lastslash - file_name] = '\0';
	  *name = (char *) lastslash + 1;
	  return
	    __hurd_file_name_lookup (use_init_port, get_dtable_port, lookup,
				     dirname, 0, 0, dir);
	}
    }
  else
    {
      /* "foobar" => cwdir + "foobar".  */
      *name = (char *) file_name;
      return (*use_init_port) (INIT_PORT_CWDIR, &addref);
    }
}
weak_alias (__hurd_file_name_split, hurd_file_name_split)

/* This is the same as hurd_file_name_split, except that it ignores
   trailing slashes (so *NAME is never "").  */
error_t
__hurd_directory_name_split (error_t (*use_init_port)
			     (int which, error_t (*operate) (file_t)),
			     file_t (*get_dtable_port) (int fd),
			     error_t (*lookup)
			     (file_t dir, char *name, int flags, mode_t mode,
			      retry_type *do_retry, string_t retry_name,
			      mach_port_t *result),
			     const char *file_name,
			     file_t *dir, char **name)
{
  error_t addref (file_t crdir)
    {
      *dir = crdir;
      return __mach_port_mod_refs (__mach_task_self (),
				   crdir, MACH_PORT_RIGHT_SEND, +1);
    }

  const char *lastslash = strrchr (file_name, '/');

  if (lastslash != NULL && lastslash[1] == '\0')
    {
      /* Trailing slash doesn't count.  Look back further.  */

      /* Back up over all trailing slashes.  */
      while (lastslash > file_name && *lastslash == '/')
	--lastslash;

      /* Find the last one earlier in the string, before the trailing ones.  */
#if __GLIBC__ > 2 || __GLIBC_MINOR__ >= 2
      lastslash = __memrchr (file_name, '/', lastslash - file_name);
#else
      /* Keep backing up, looking for a slash.  */
      do
	if (lastslash == file_name)
	  {
	    /* Hit the start with no slash.  */
	    lastslash = NULL;
	    break;
	  }
      while (*lastslash-- != '/');
#endif
    }

  if (lastslash != NULL)
    {
      if (lastslash == file_name)
	{
	  /* "/foobar" => crdir + "foobar".  */
	  *name = (char *) file_name + 1;
	  return (*use_init_port) (INIT_PORT_CRDIR, &addref);
	}
      else
	{
	  /* "/dir1/dir2/.../file".  */
	  char dirname[lastslash - file_name + 1];
	  memcpy (dirname, file_name, lastslash - file_name);
	  dirname[lastslash - file_name] = '\0';
	  *name = (char *) lastslash + 1;
	  return
	    __hurd_file_name_lookup (use_init_port, get_dtable_port, lookup,
				     dirname, 0, 0, dir);
	}
    }
  else
    {
      /* "foobar" => cwdir + "foobar".  */
      *name = (char *) file_name;
      return (*use_init_port) (INIT_PORT_CWDIR, &addref);
    }
}
weak_alias (__hurd_directory_name_split, hurd_directory_name_split)


file_t
__file_name_lookup (const char *file_name, int flags, mode_t mode)
{
  error_t err;
  file_t result;

  err = __hurd_file_name_lookup (&_hurd_ports_use, &__getdport, 0,
				 file_name, flags, mode & ~_hurd_umask,
				 &result);

  return err ? (__hurd_fail (err), MACH_PORT_NULL) : result;
}
weak_alias (__file_name_lookup, file_name_lookup)


file_t
__file_name_split (const char *file_name, char **name)
{
  error_t err;
  file_t result;

  err = __hurd_file_name_split (&_hurd_ports_use, &__getdport, 0,
				file_name, &result, name);

  return err ? (__hurd_fail (err), MACH_PORT_NULL) : result;
}
weak_alias (__file_name_split, file_name_split)

file_t
__directory_name_split (const char *directory_name, char **name)
{
  error_t err;
  file_t result;

  err = __hurd_directory_name_split (&_hurd_ports_use, &__getdport, 0,
				     directory_name, &result, name);

  return err ? (__hurd_fail (err), MACH_PORT_NULL) : result;
}
weak_alias (__directory_name_split, directory_name_split)


file_t
__file_name_lookup_under (file_t startdir,
			  const char *file_name, int flags, mode_t mode)
{
  error_t err;
  file_t result;

  error_t use_init_port (int which, error_t (*operate) (mach_port_t))
    {
      return (which == INIT_PORT_CWDIR ? (*operate) (startdir) :
	      _hurd_ports_use (which, operate));
    }

  err = __hurd_file_name_lookup (&use_init_port, &__getdport, 0,
				 file_name, flags, mode & ~_hurd_umask,
				 &result);

  return err ? (__hurd_fail (err), MACH_PORT_NULL) : result;
}
weak_alias (__file_name_lookup_under, file_name_lookup_under)
