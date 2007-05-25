/* Checking macros for fcntl functions.
   Copyright (C) 2007 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef	_FCNTL_H
# error "Never include <bits/fcntl2.h> directly; use <fcntl.h> instead."
#endif

/* Check that calls to open and openat with O_CREAT set have an
   appropriate third/fourth parameter.  */
#ifndef __USE_FILE_OFFSET64
extern int __open_2 (__const char *__path, int __oflag) __nonnull ((1));
#else
extern int __REDIRECT (__open_2, (__const char *__file, int __oflag),
		       __open64_2) __nonnull ((1));
#endif

#define open(fname, flags, ...) \
  ({ int ___r;								      \
     /* If the compiler complains about an invalid type, excess elements, etc \
	in the initialization this means a parameter of the wrong type has    \
	been passed to open. */						      \
     int ___arr[] = { __VA_ARGS__ };					      \
     if (__builtin_constant_p (flags) && ((flags) & O_CREAT) != 0)	      \
       {								      \
	 /* If the compiler complains about the size of this array type the   \
	    mode parameter is missing since O_CREAT has been used.  */	      \
	 typedef int __open_missing_mode[((flags) & O_CREAT) != 0	      \
					 ? ((long int) sizeof (___arr)	      \
					    - (long int) sizeof (int)) : 1];  \
       }								      \
     if (sizeof (___arr) == 0)						      \
       {								      \
	 if (__builtin_constant_p (flags) && ((flags) & O_CREAT) == 0)	      \
	   ___r = open (fname, flags);					      \
	 else     							      \
	   ___r = __open_2 (fname, flags);				      \
       }								      \
     else								      \
       {								      \
	 /* If the compiler complains about the size of this array type too   \
	    many parameters have been passed to open.  */		      \
	 typedef int __open_too_many_args[-(sizeof (___arr) > sizeof (int))]; \
	 ___r = open (fname, flags, ___arr[0]);				      \
       }								      \
     ___r;								      \
  })


#ifdef __USE_LARGEFILE64
extern int __open64_2 (__const char *__path, int __oflag) __nonnull ((1));

# define open64(fname, flags, ...) \
  ({ int ___r;								      \
     /* If the compiler complains about an invalid type, excess elements, etc \
	in the initialization this means a parameter of the wrong type has    \
	been passed to open64. */					      \
     int ___arr[] = { __VA_ARGS__ };					      \
     if (__builtin_constant_p (flags) && ((flags) & O_CREAT) != 0)	      \
       {								      \
	 /* If the compiler complains about the size of this array type the   \
	    mode parameter is missing since O_CREAT has been used.  */	      \
	 typedef int __open_missing_mode[((flags) & O_CREAT) != 0	      \
					 ? ((long int) sizeof (___arr)	      \
					    - (long int) sizeof (int)) : 1];  \
       }								      \
     if (sizeof (___arr) == 0)						      \
       {								      \
	 if (__builtin_constant_p (flags) && ((flags) & O_CREAT) == 0)	      \
	   ___r = open64 (fname, flags);				      \
	 else     							      \
	   ___r = __open64_2 (fname, flags);				      \
       }								      \
     else								      \
       {								      \
	 /* If the compiler complains about the size of this array type too   \
	    many parameters have been passed to open64.  */		      \
	 typedef int __open_too_many_args[-(sizeof (___arr) > sizeof (int))]; \
	 ___r = open64 (fname, flags, ___arr[0]);			      \
       }								      \
     ___r;								      \
  })
#endif

#ifdef __USE_ATFILE
# ifndef __USE_FILE_OFFSET64
extern int __openat_2 (int __fd, __const char *__path, int __oflag)
     __nonnull ((2));
# else
extern int __REDIRECT (__openat_2, (int __fd, __const char *__file,
				    int __oflag), __openat64_2)
     __nonnull ((2));
# endif

# define openat(fd, fname, flags, ...) \
  ({ int ___r;								      \
     /* If the compiler complains about an invalid type, excess elements, etc \
	in the initialization this means a parameter of the wrong type has    \
	been passed to openat. */					      \
     int ___arr[] = { __VA_ARGS__ };					      \
     if (__builtin_constant_p (flags) && ((flags) & O_CREAT) != 0)	      \
       {								      \
	 /* If the compiler complains about the size of this array type the   \
	    mode parameter is missing since O_CREAT has been used.  */	      \
	 typedef int __open_missing_mode[((flags) & O_CREAT) != 0	      \
					 ? ((long int) sizeof (___arr)	      \
					    - (long int) sizeof (int)) : 1];  \
       }								      \
     if (sizeof (___arr) == 0)						      \
       {								      \
	 if (__builtin_constant_p (flags) && ((flags) & O_CREAT) == 0)	      \
	   ___r = openat (fd, fname, flags);				      \
	 else     							      \
	   ___r = __openat_2 (fd, fname, flags);			      \
       }								      \
     else								      \
       {								      \
	 /* If the compiler complains about the size of this array type too   \
	    many parameters have been passed to openat.  */		      \
	 typedef int __open_too_many_args[-(sizeof (___arr) > sizeof (int))]; \
	 ___r = openat (fd, fname, flags, ___arr[0]);			      \
       }								      \
     ___r;								      \
  })


# ifdef __USE_LARGEFILE64
extern int __openat64_2 (int __fd, __const char *__path, int __oflag)
     __nonnull ((2));

#  define openat64(fd, fname, flags, ...) \
  ({ int ___r;								      \
     /* If the compiler complains about an invalid type, excess elements, etc \
	in the initialization this means a parameter of the wrong type has    \
	been passed to openat64. */					      \
     int ___arr[] = { __VA_ARGS__ };					      \
     if (__builtin_constant_p (flags) && ((flags) & O_CREAT) != 0)	      \
       {								      \
	 /* If the compiler complains about the size of this array type the   \
	    mode parameter is missing since O_CREAT has been used.  */	      \
	 typedef int __open_missing_mode[((flags) & O_CREAT) != 0	      \
					 ? ((long int) sizeof (___arr)	      \
					    - (long int) sizeof (int)) : 1];  \
       }								      \
     if (sizeof (___arr) == 0)						      \
       {								      \
	 if (__builtin_constant_p (flags) && ((flags) & O_CREAT) == 0)	      \
	   ___r = openat64 (fd, fname, flags);				      \
	 else     							      \
	   ___r = __openat64_2 (fd, fname, flags);			      \
       }								      \
     else								      \
       {								      \
	 /* If the compiler complains about the size of this array type too   \
	    many parameters have been passed to openat64.  */		      \
	 typedef int __open_too_many_args[-(sizeof (___arr) > sizeof (int))]; \
	 ___r = openat64 (fd, fname, flags, ___arr[0]);			      \
       }								      \
     ___r;								      \
  })
# endif
#endif
