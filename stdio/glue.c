/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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

/* This file provides glue between Unix stdio and GNU stdio.
   It supports use of Unix stdio `getc' and `putc' (and, by extension,
   `getchar' and `putchar') macros on GNU stdio streams (they are slow, but
   they work).  It also supports all stdio operations (including Unix
   `getc' and `putc') on Unix's stdin, stdout, and stderr (the elements of
   `_iob').

   The reasoning behind this is to allow programs (and especially
   libraries) compiled with Unix header files to work with the GNU C
   library.  */

#include <ansidecl.h>
#include <stdio.h>
#include <errno.h>

typedef union
  {
    struct
      {
	int magic;
	FILE **streamp;		/* Overlaps GNU stdio `bufp' member.  */
	/* These two overlap the GNU stdio `get_limit' and `put_limit'
	   members.  They must be <= `streamp'/`bufp' for GNU getc and putc
	   to do the right thing.  */
	FILE **streamp2, **streamp3;
      } glue;
    struct _iobuf
      {
	int _cnt;
	unsigned char *_ptr;
	unsigned char *_base;
	int _bufsiz;
	short int _flag;
	char _file;
      } unix_iobuf;
    FILE gnu_stream;
  } unix_FILE;

/* These are the Unix stdio's stdin, stdout, and stderr.
   In Unix stdin is (&_iob[0]), stdout is (&_iob[1]), and stderr is
   (&_iob[2]).  The magic number marks these as glued streams.  The
   __validfp macro in stdio.h is used by every stdio function.  It checks
   for glued streams, and replaces them with the GNU stdio stream.  */
unix_FILE _iob[] =
  {
#define	S(name)	{ { _GLUEMAGIC, &name, &name, &name } }
    S (stdin),
    S (stdout),
    S (stderr),
#undef	S
  };

/* Called by the Unix stdio `getc' macro.
   The macro is assumed to look something like:
       (--file->_cnt < 0 ? _filbuf (file) ...)
   In a Unix stdio FILE `_cnt' is the first element.
   In a GNU stdio or glued FILE, the first element is the magic number.  */
int
DEFUN(_filbuf, (file), unix_FILE *file)
{
  switch (++file->glue.magic)	/* Compensate for Unix getc's decrement.  */
    {
    case _GLUEMAGIC:
      /* This is a glued stream.  */
      return getc (*file->glue.streamp);

    case  _IOMAGIC:
      /* This is a normal GNU stdio stream.  */
      return getc ((FILE *) file);

    default:
      /* Bogus stream.  */
      errno = EINVAL;
      return EOF;
    }
}

/* Called by the Unix stdio `putc' macro.  Much like getc, above.  */
int
DEFUN(_flsbuf, (c, file),
      int c AND unix_FILE *file)
{
  /* Compensate for putc's decrement.  */
  switch (++file->glue.magic)
    {
    case _GLUEMAGIC:
      return putc (c, *file->glue.streamp);

    case  _IOMAGIC:
      return putc (c, (FILE *) file);

    default:
      errno = EINVAL;
      return EOF;
    }
}
