/* Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.

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

#include "localeinfo.h"

/* This table's entries are taken from POSIX.2 Table 2-11
   ``LC_TIME Category Definition in the POSIX Locale''.  */

const struct locale_data _nl_C_LC_TIME =
{
  _nl_C_name,
  NULL, 0, 0, /* no file mapped */
  UNDELETABLE,
  54,
  {
    { string: "Sun" },
    { string: "Mon" },
    { string: "Tue" },
    { string: "Wed" },
    { string: "Thu" },
    { string: "Fri" },
    { string: "Sat" },
    { string: "Sunday" },
    { string: "Monday" },
    { string: "Tuesday" },
    { string: "Wednesday" },
    { string: "Thursday" },
    { string: "Friday" },
    { string: "Saturday" },
    { string: "Jan" },
    { string: "Feb" },
    { string: "Mar" },
    { string: "Apr" },
    { string: "May" },
    { string: "Jun" },
    { string: "Jul" },
    { string: "Aug" },
    { string: "Sep" },
    { string: "Oct" },
    { string: "Nov" },
    { string: "Dec" },
    { string: "January" },
    { string: "February" },
    { string: "March" },
    { string: "April" },
    { string: "May" },
    { string: "June" },
    { string: "July" },
    { string: "August" },
    { string: "September" },
    { string: "October" },
    { string: "November" },
    { string: "December" },
    { string: "AM" },
    { string: "PM" },
    { string: "%a %b %e %H:%M:%S %Y" },
    { string: "%m/%d/%y" },
    { string: "%H:%M:%S" },
    { string: "%I:%M:%S %p" },
    { string: NULL },
    { string: "" },
    { string: "" },
    { string: "" },
    { string: "" },
    { string: "" },
    { word: 0 },
    { word: 0 },
    { string: "" },
    { string: "" },
  }
};
