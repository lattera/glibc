/* Copyright (C) 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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

#include <hurd.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include "stdio/_itoa.h"
#include <hurd/term.h>


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
__hurd_file_name_lookup (file_t crdir, file_t cwdir,
			 const char *file_name, int flags, mode_t mode,
			 file_t *result)
{
  error_t err;
  enum retry_type doretry;
  char retryname[1024];		/* XXX string_t LOSES! */
  file_t startdir;

  startdir = file_name[0] == '/' ? crdir : cwdir;

  while (file_name[0] == '/')
    file_name++;

  if (err = __dir_lookup (startdir, file_name, flags, mode,
			  &doretry, retryname, result))
    return lookup_error (err);

  return __hurd_file_name_lookup_retry (crdir, doretry, retryname, flags, mode,
					result);
}
weak_alias (__hurd_file_name_lookup, hurd_file_name_lookup)

error_t
__hurd_file_name_lookup_retry (file_t crdir,
			       enum retry_type doretry,
			       char retryname[1024],
			       int flags, mode_t mode,
			       file_t *result)
{
  error_t err;
  file_t startdir;
  file_t newpt;
  char *file_name;
  int dealloc_dir;
  int nloops;

  dealloc_dir = 0;
  nloops = 0;
  err = 0;
  
  while (1)
    {
      if (dealloc_dir)
	__mach_port_deallocate (__mach_task_self (), startdir);
      if (err)
	return lookup_error (err);

      switch (doretry)
	{
	case FS_RETRY_REAUTH:
	  {
	    mach_port_t ref = __mach_reply_port ();
	    err = __io_reauthenticate (*result,
				       ref, MACH_MSG_TYPE_MAKE_SEND);
	    if (! err)
	      err = __USEPORT
		(AUTH, __auth_user_authenticate (port, *result,
						 ref,
						 MACH_MSG_TYPE_MAKE_SEND,
						 &newpt));
	    __mach_port_destroy (__mach_task_self (), ref);
	  }
	  __mach_port_deallocate (__mach_task_self (), *result);
	  if (err)
	    return err;
	  *result = newpt;
	  /* Fall through.  */

	case FS_RETRY_NORMAL:
#ifdef SYMLOOP_MAX
	  if (nloops++ >= SYMLOOP_MAX)
	    return ELOOP;
#endif

	  /* An empty RETRYNAME indicates we have the final port.  */
	  if (retryname[0] == '\0')
	    {
	      /* We got a successful translation.  Now apply any open-time
		 action flags we were passed.  */
	      if (flags & O_EXLOCK)
		;		/* XXX */
	      if (!err && (flags & O_SHLOCK))
		;		/* XXX */
	      if (!err && (flags & O_TRUNC))
		err = __file_set_size (*result, 0);

	      if (err)
		__mach_port_deallocate (__mach_task_self (), *result);
	      return err;
	    }

	  startdir = *result;
	  dealloc_dir = 1;
	  file_name = retryname;
	  break;

	case FS_RETRY_MAGICAL:
	  switch (retryname[0])
	    {
	    case '/':
	      startdir = crdir;
	      dealloc_dir = 0;
	      if (*result != MACH_PORT_NULL)
		__mach_port_deallocate (__mach_task_self (), *result);
	      file_name = &retryname[1];
	      break;

	    case 'f':
	      if (retryname[1] == 'd' && retryname[2] == '/')
		{
		  int fd;
		  char *end;
		  int save = errno;
		  errno = 0;
		  fd = (int) strtol (retryname, &end, 10);
		  if (end == NULL || errno || /* Malformed number.  */
		      /* Check for excess text after the number.  A slash
			 is valid; it ends the component.  Anything else
			 does not name a numeric file descriptor.  */
		      (*end != '/' && *end != '\0'))
		    {
		      errno = save;
		      return ENOENT;
		    }
		  *result = __getdport (fd);
		  if (*result == MACH_PORT_NULL)
		    {
		      /* If the name was a proper number, but the file
			 descriptor does not exist, we return EBADF instead
			 of ENOENT.  */
		      error_t err = errno;
		      errno = save;
		      return err;
		    }
		  errno = save;
		  if (*end == '\0')
		    return 0;
		  else
		    {
		      /* Do a normal retry on the remaining components.  */
		      startdir = *result;
		      dealloc_dir = 1;
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
		  dealloc_dir = 1;
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
			file_t unauth;
			err = __USEPORT (CTTYID,
					 __termctty_open_terminal (port,
								   flags,
								   &unauth));
			if (! err)
			  {
			    mach_port_t ref = __mach_reply_port ();
			    err = __io_reauthenticate
			      (unauth,
			       ref,
			       MACH_MSG_TYPE_MAKE_SEND);
			    if (! err)
			      err = __USEPORT
				(AUTH, __auth_user_authenticate
				 (port,
				  unauth,
				  ref, MACH_MSG_TYPE_MAKE_SEND,
				  result));
			    __mach_port_deallocate (__mach_task_self (),
						    unauth);
			    __mach_port_destroy (__mach_task_self (), ref);
			  }
			return err;
		      }

		  case '\0':
		    return opentty (result);
		  case '/':
		    if (err = opentty (&startdir))
		      return err;
		    dealloc_dir = 1;
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

      err = __dir_lookup (startdir, file_name, flags, mode,
			  &doretry, retryname, result);
    }
}
weak_alias (__hurd_file_name_lookup_retry, hurd_file_name_lookup_retry)

error_t
__hurd_file_name_split (file_t crdir, file_t cwdir,
			const char *file_name,
			file_t *dir, char **name)
{
  const char *lastslash;
  error_t err;

  lastslash = strrchr (file_name, '/');
  if (lastslash != NULL)
    {
      if (lastslash == file_name)
	{
	  /* "/foobar" => crdir + "foobar".  */
	  *name = (char *) file_name + 1;
	  if (err = __mach_port_mod_refs (__mach_task_self (), 
					  crdir, MACH_PORT_RIGHT_SEND, +1))
	    return err;
	  *dir = crdir;
	  return 0;
	}
      else
	{
	  /* "/dir1/dir2/.../file".  */
	  char dirname[lastslash - file_name + 1];
	  memcpy (dirname, file_name, lastslash - file_name);
	  dirname[lastslash - file_name] = '\0';
	  *name = (char *) lastslash + 1;
	  return __hurd_file_name_lookup (crdir, cwdir, dirname, 0, 0, dir);
	}
    }
  else
    {
      /* "foobar" => cwdir + "foobar".  */
      *name = (char *) file_name;
      if (err = __mach_port_mod_refs (__mach_task_self (),
				      cwdir, MACH_PORT_RIGHT_SEND, +1))
	return err;
      *dir = cwdir;
      return 0;
    }
}
weak_alias (__hurd_file_name_split, hurd_file_name_split)


file_t
__file_name_lookup (const char *file_name, int flags, mode_t mode)
{
  error_t err;
  file_t result, crdir, cwdir;
  struct hurd_userlink crdir_ulink, cwdir_ulink;

  crdir = _hurd_port_get (&_hurd_ports[INIT_PORT_CRDIR], &crdir_ulink);
  cwdir = _hurd_port_get (&_hurd_ports[INIT_PORT_CWDIR], &cwdir_ulink);

  err = __hurd_file_name_lookup (crdir, cwdir, file_name, flags, mode,
				 &result);

  _hurd_port_free (&_hurd_ports[INIT_PORT_CRDIR], &crdir_ulink, crdir);
  _hurd_port_free (&_hurd_ports[INIT_PORT_CWDIR], &cwdir_ulink, cwdir);

  if (err)
    return __hurd_fail (err), MACH_PORT_NULL;
  else
    return result;
}
weak_alias (__file_name_lookup, file_name_lookup)


file_t
__file_name_split (const char *file_name, char **name)
{
  error_t err;
  file_t dir, crdir, cwdir;
  struct hurd_userlink crdir_ulink, cwdir_ulink;

  crdir = _hurd_port_get (&_hurd_ports[INIT_PORT_CRDIR], &crdir_ulink);
  cwdir = _hurd_port_get (&_hurd_ports[INIT_PORT_CWDIR], &cwdir_ulink);

  err = __hurd_file_name_split (crdir, cwdir, file_name, &dir, name);

  _hurd_port_free (&_hurd_ports[INIT_PORT_CRDIR], &crdir_ulink, crdir);
  _hurd_port_free (&_hurd_ports[INIT_PORT_CWDIR], &cwdir_ulink, cwdir);

  if (err)
    {
      errno = err;
      return MACH_PORT_NULL;
    }
  else
    return dir;
}
weak_alias (__file_name_split, file_name_split)
