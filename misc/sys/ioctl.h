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
not, write to the, 1992 Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef	_SYS_IOCTL_H

#define	_SYS_IOCTL_H	1
#include <features.h>

__BEGIN_DECLS

/* Get the list of `ioctl' requests and related constants.  */
#include <ioctls.h>

/* On a Unix system, the system <sys/ioctl.h> probably defines some of the
   symbols we define in <sys/ttydefaults.h> (usually with the same values).
   The code to generate <ioctls.h> has omitted these symbols to avoid the
   conflict, but a Unix program expects <sys/ioctl.h> to define them, so we
   must include <sys/ttydefaults.h> here.  */
#include <sys/ttydefaults.h>

#if	defined(TIOCGETC) || defined(TIOCSETC)
/* Type of ARG for TIOCGETC and TIOCSETC requests.  */
struct tchars
{
  char t_intrc;			/* Interrupt character.  */
  char t_quitc;			/* Quit character.  */
  char t_startc;		/* Start-output character.  */
  char t_stopc;			/* Stop-output character.  */
  char t_eofc;			/* End-of-file character.  */
  char t_brkc;			/* Input delimiter character.  */
};

#define	_IOT_tchars	/* Hurd ioctl type field.  */ \
  _IOT (_IOTS (char), 6, 0, 0, 0, 0)
#endif

#if	defined(TIOCGLTC) || defined(TIOCSLTC)
/* Type of ARG for TIOCGLTC and TIOCSLTC requests.  */
struct ltchars
{
  char t_suspc;			/* Suspend character.  */
  char t_dsuspc;		/* Delayed suspend character.  */
  char t_rprntc;		/* Reprint-line character.  */
  char t_flushc;		/* Flush-output character.  */
  char t_werasc;		/* Word-erase character.  */
  char t_lnextc;		/* Literal-next character.  */
};

#define	_IOT_ltchars	/* Hurd ioctl type field.  */ \
  _IOT (_IOTS (char), 6, 0, 0, 0, 0)
#endif

/* Type of ARG for TIOCGETP and TIOCSETP requests (and gtty and stty).  */
struct sgttyb
{
  char sg_ispeed;		/* Input speed.  */
  char sg_ospeed;		/* Output speed.  */
  char sg_erase;		/* Erase character.  */
  char sg_kill;			/* Kill character.  */
  short int sg_flags;		/* Mode flags.  */
};

#define	_IOT_sgttyb	/* Hurd ioctl type field.  */ \
  _IOT (_IOTS (char), 6, _IOTS (short int), 1, 0, 0)

#if	defined(TIOCGWINSZ) || defined(TIOCSWINSZ)
/* Type of ARG for TIOCGWINSZ and TIOCSWINSZ requests.  */
struct winsize
{
  unsigned short int ws_row;	/* Rows, in characters.  */
  unsigned short int ws_col;	/* Columns, in characters.  */

  /* These are not actually used.  */
  unsigned short int ws_xpixel;	/* Horizontal pixels.  */
  unsigned short int ws_ypixel;	/* Vertical pixels.  */
};

#define	_IOT_winsize	/* Hurd ioctl type field.  */ \
  _IOT (_IOTS (unsigned short int), 4, 0, 0, 0, 0)
#endif

#if	defined (TIOCGSIZE) || defined (TIOCSSIZE)
#  if defined (TIOCGWINSZ) && TIOCGSIZE == TIOCGWINSZ
/* Many systems that have TIOCGWINSZ define TIOCGSIZE for source
   compatibility with Sun; they define `struct ttysize' to have identical
   layout as `struct winsize' and #define TIOCGSIZE to be TIOCGWINSZ
   (likewise TIOCSSIZE and TIOCSWINSZ).  */
struct ttysize
{
  unsigned short int ts_lines;
  unsigned short int ts_cols;
  unsigned short int ts_xxx;
  unsigned short int ts_yyy;
};
#define	_IOT_ttysize	_IOT_winsize
#  else
/* Suns use a different layout for `struct ttysize', and TIOCGSIZE and
   TIOCGWINSZ are separate commands that do the same thing with different
   structures (likewise TIOCSSIZE and TIOCSWINSZ).  */
struct ttysize
{
  int ts_lines, ts_cols;	/* Lines and columns, in characters.  */
};
#  endif
#endif

/* Perform the I/O control operation specified by REQUEST on FD.
   One argument may follow; its presence and type depend on REQUEST.
   Return value depends on REQUEST.  Usually -1 indicates error.  */
extern int __ioctl __P ((int __fd, unsigned long int __request, ...));
extern int ioctl __P ((int __fd, unsigned long int __request, ...));

__END_DECLS

#endif /* sys/ioctl.h */
