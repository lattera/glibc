/* Copyright (C) 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger <davidm@cs.arizona.edu>.

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

/* This file specifies the format of gmon.out files.  It should have
   as few external dependencies as possible as it is going to be included
   in many different programs.  That is, minimize the number of #include's.

   A gmon.out file consists of a header (defined by gmon_hdr) followed by
   a sequence of records.  Each record starts with a one-byte tag
   identifying the type of records, followed by records specific data. */

#ifndef _SYS_GMON_OUT_H

#define _SYS_GMON_OUT_H	1
#include <features.h>

#define	GMON_MAGIC	"gmon"	/* magic cookie */
#define GMON_VERSION	1	/* version number */

__BEGIN_DECLS

/*
 * Raw header as it appears on file (without padding).  This header
 * always comes first in gmon.out and is then followed by a series
 * records defined below.
 */
struct gmon_hdr {
  char cookie[4];
  int version;
  int spare[3];
};

/* types of records in this file: */
typedef enum {
  GMON_TAG_TIME_HIST = 0, GMON_TAG_CG_ARC = 1, GMON_TAG_BB_COUNT = 2
} GMON_Record_Tag;

struct gmon_hist_hdr {
  unsigned long low_pc;			/* base pc address of sample buffer */
  unsigned long high_pc;		/* max pc address of sampled buffer */
  int hist_size;			/* size of sample buffer */
  int prof_rate;			/* profiling clock rate */
  char dimen[15];			/* phys. dim., usually "seconds" */
  char dimen_abbrev;			/* usually 's' for "seconds" */
};

struct gmon_cg_arc_record {
  unsigned long from_pc;		/* address within caller's body */
  unsigned long self_pc;		/* address within callee's body */
  int count;				/* number of arc traversals */
};

__END_DECLS

#endif /* _SYS_GMON_OUT_H */
