/* Copyright (C) 1991, 1992, 1993, 1994 Free Software Foundation, Inc.
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

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <hurd.h>
#include <hurd/port.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>


/* Get the pathname of the current working directory, and put it in SIZE
   bytes of BUF.  Returns NULL if the directory couldn't be determined or
   SIZE was too small.  If successful, returns BUF.  In GNU, if BUF is
   NULL, an array is allocated with `malloc'; the array is SIZE bytes long,
   unless SIZE <= 0, in which case it is as big as necessary.  */

char *
getcwd (char *buf, size_t size)
{
  error_t err;
  dev_t rootdev, thisdev;
  ino_t rootino, thisino;
  char *file_name;
  register char *file_namep;
  struct stat st;
  file_t parent;
  char *dirbuf = NULL;
  unsigned int dirbufsize = 0;
  file_t crdir;
  struct hurd_userlink crdir_ulink;

  inline void cleanup (void)
    {
      _hurd_port_free (&_hurd_ports[INIT_PORT_CRDIR], &crdir_ulink, crdir);
      __mach_port_deallocate (__mach_task_self (), parent);

      if (dirbuf != NULL)
	__vm_deallocate (__mach_task_self (),
			 (vm_address_t) dirbuf, dirbufsize);
    }

      
  if (size == 0)
    {
      if (buf != NULL)
	{
	  errno = EINVAL;
	  return NULL;
	}

      size = FILENAME_MAX * 4 + 1;	/* Good starting guess.  */
    }

  if (buf != NULL)
    file_name = buf;
  else
    {
      file_name = malloc (size);
      if (file_name == NULL)
	return NULL;
    }

  file_namep = file_name + size;
  *--file_namep = '\0';

  /* Get a port to our root directory and stat it.  */

  crdir = _hurd_port_get (&_hurd_ports[INIT_PORT_CRDIR], &crdir_ulink);
  if (err = __io_stat (crdir, &st))
    {
      _hurd_port_free (&_hurd_ports[INIT_PORT_CRDIR], &crdir_ulink, crdir);
      return __hurd_fail (err), NULL;
    }
  rootdev = st.st_dev;
  rootino = st.st_ino;

  /* Get a port to our current working directory and stat it.  */

  if (err = __USEPORT (CWDIR, __mach_port_mod_refs (__mach_task_self (),
						    (parent = port),
						    MACH_PORT_RIGHT_SEND,
						    1)))
    {
      _hurd_port_free (&_hurd_ports[INIT_PORT_CRDIR], &crdir_ulink, crdir);
      return __hurd_fail (err), NULL;
    }
  if (err = __io_stat (parent, &st))
    {
      cleanup ();
      return __hurd_fail (err), NULL;
    }

  thisdev = st.st_dev;
  thisino = st.st_ino;

  while (!(thisdev == rootdev && thisino == rootino))
    {
      /* PARENT is a port to the directory we are currently on;
	 THISDEV and THISINO are its device and node numbers.
	 Look in its parent (..) for a file with the same numbers.  */

      struct dirent *d;
      dev_t dotdev;
      ino_t dotino;
      int mount_point;
      file_t newp;
      char *dirdata;
      unsigned int dirdatasize;
      int direntry, nentries;

      /* Look at the parent directory.  */
      if (err = __hurd_file_name_lookup (crdir, parent, "..", O_READ, 0, &newp))
	goto lose;
      __mach_port_deallocate (__mach_task_self (), parent);
      parent = newp;

      /* Figure out if this directory is a mount point.  */
      if (err = __io_stat (parent, &st))
	goto lose;
      dotdev = st.st_dev;
      dotino = st.st_ino;
      mount_point = dotdev != thisdev;

      /* Search for the last directory.  */
      direntry = 0;
      dirdata = dirbuf;
      dirdatasize = dirbufsize;
      while (!(err = __dir_readdir (parent, &dirdata, &dirdatasize,
				    direntry, -1, 0, &nentries)) &&
	     nentries != 0)	     
	{
	  /* We have a block of directory entries.  */

	  unsigned int offset;

	  direntry += nentries;

	  if (dirdata != dirbuf)
	    {
	      /* The data was passed out of line, so our old buffer is no
		 longer useful.  Deallocate the old buffer and reset our
		 information for the new buffer.  */
	      __vm_deallocate (__mach_task_self (),
			       (vm_address_t) dirbuf, dirbufsize);
	      dirbuf = dirdata;
	      dirbufsize = round_page (dirdatasize);
	    }

	  /* Iterate over the returned directory entries, looking for one
	     whose file number is THISINO.  */

	  offset = 0;
	  while (offset < dirdatasize)
	    {
	      d = (struct dirent *) &dirdata[offset];
	      offset += d->d_reclen;

	      /* Ignore `.' and `..'.  */	
	      if (d->d_name[0] == '.' &&
		  (d->d_namlen == 1 ||
		   (d->d_namlen == 2 && d->d_name[1] == '.')))
		continue;

	      if (mount_point || d->d_ino == thisino)
		{
		  file_t try;
		  if (err = __hurd_file_name_lookup (crdir, parent, d->d_name,
						     O_NOLINK, 0, &try))
		    goto lose;
		  err = __io_stat (try, &st);
		  __mach_port_deallocate (__mach_task_self (), try);
		  if (err)
		    goto lose;
		  if (st.st_dev == thisdev && st.st_ino == thisino)
		    break;
		}
	    }
	}

      if (err)
	goto lose;
      else
	{
	  /* Prepend the directory name just discovered.  */

	  if (file_namep - file_name < d->d_namlen + 1)
	    {
	      if (buf != NULL)
		{
		  errno = ERANGE;
		  return NULL;
		}
	      else
		{
		  size *= 2;
		  buf = realloc (file_name, size);
		  if (buf == NULL)
		    {
		      free (file_name);
		      return NULL;
		    }
		  file_namep = &buf[file_namep - file_name];
		  file_name = buf;
		}
	    }
	  file_namep -= d->d_namlen;
	  (void) memcpy (file_namep, d->d_name, d->d_namlen);
	  *--file_namep = '/';
	}

      /* The next iteration will find the name of the directory we
	 just searched through.  */
      thisdev = dotdev;
      thisino = dotino;
    }

  if (file_namep == &file_name[size - 1])
    /* We found nothing and got all the way to the root.
       So the root is our current directory.  */
    *--file_namep = '/';

  memmove (file_name, file_namep, file_name + size - file_namep);
  cleanup ();
  return file_name;

 lose:
  cleanup ();
  return NULL;
}
