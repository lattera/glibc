/* Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@uni-paderborn.de>, 1998.

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

#ifndef _NSCD_PROTO_H
#define _NSCD_PROTO_H 1

#include <grp.h>
#include <pwd.h>

extern int __nscd_getpwnam_r __P ((const char *name, struct passwd *resultbuf,
				   char *buffer, size_t buflen));
extern int __nscd_getpwuid_r __P ((uid_t uid, struct passwd *resultbuf,
				   char *buffer,  size_t buflen));
extern int __nscd_getgrnam_r __P ((const char *name, struct group *resultbuf,
				   char *buffer, size_t buflen));
extern int __nscd_getgrgid_r __P ((uid_t uid, struct group *resultbuf,
				   char *buffer,  size_t buflen));

#endif /* _NSCD_PROTO_H */
