/* Declarations of file name translation functions for the GNU Hurd.
Copyright (C) 1995 Free Software Foundation, Inc.
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

#ifndef _HURD_LOOKUP_H
#define _HURD_LOOKUP_H	1

/* These functions all take two callback functions as the first two arguments.
   The first callback function USE_INIT_PORT is called as follows:

   error_t use_init_port (int which, error_t (*operate) (mach_port_t));

   WHICH is nonnegative value less than INIT_PORT_MAX, indicating which
   init port is required.  The callback function should call *OPERATE
   with a send right to the appropriate init port.  No user reference
   is consumed; the right will only be used after *OPERATE returns if
   *OPERATE has added its own user reference.

   The second callback function GET_DTABLE_PORT should behave like `getdport'.

   All these functions return zero on success or an error code on failure.  */


/* Open a port to FILE with the given FLAGS and MODE (see <fcntl.h>).  If
   successful, returns zero and store the port to FILE in *PORT; otherwise
   returns an error code. */

error_t __hurd_file_name_lookup (error_t (*use_init_port)
				   (int which,
				    error_t (*operate) (mach_port_t)),
				 file_t (*get_dtable_port) (int fd),
				 const char *file_name,
				 int flags, mode_t mode,
				 file_t *result);
error_t hurd_file_name_lookup (error_t (*use_init_port)
			         (int which,
				  error_t (*operate) (mach_port_t)),
			       file_t (*get_dtable_port) (int fd),
			       const char *file_name,
			       int flags, mode_t mode,
			       file_t *result);


/* Split FILE into a directory and a name within the directory.  Look up a
   port for the directory and store it in *DIR; store in *NAME a pointer
   into FILE where the name within directory begins.  */

error_t __hurd_file_name_split (error_t (*use_init_port)
				  (int which,
				   error_t (*operate) (mach_port_t)),
				file_t (*get_dtable_port) (int fd),
				const char *file_name,
				file_t *dir, char **name);
error_t hurd_file_name_split (error_t (*use_init_port)
			        (int which,
				 error_t (*operate) (mach_port_t)),
			      file_t (*get_dtable_port) (int fd),
			      const char *file_name,
			      file_t *dir, char **name);


/* Process the values returned by `dir_lookup' et al, and loop doing
   `dir_lookup' calls until one returns FS_RETRY_NONE.  The arguments
   should be those just passed to and/or returned from `dir_lookup',
   `fsys_getroot', or `file_invoke_translator'.  This function consumes the
   reference in *RESULT even if it returns an error.  */

error_t __hurd_file_name_lookup_retry (error_t (*use_init_port)
				         (int which,
					  error_t (*operate) (mach_port_t)),
				       file_t (*get_dtable_port) (int fd),
				       enum retry_type doretry,
				       char retryname[1024],
				       int flags, mode_t mode,
				       file_t *result);
error_t hurd_file_name_lookup_retry (error_t (*use_init_port)
				       (int which,
					error_t (*operate) (mach_port_t)),
				     file_t (*get_dtable_port) (int fd),
				     enum retry_type doretry,
				     char retryname[1024],
				     int flags, mode_t mode,
				     file_t *result);


#endif	/* hurd/lookup.h */
