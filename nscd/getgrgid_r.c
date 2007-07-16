/* Copyright (C) 1996, 1997, 1998, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include <grp.h>


#define LOOKUP_TYPE	struct group
#define FUNCTION_NAME	getgrgid
#define DATABASE_NAME	group
#define ADD_PARAMS	gid_t gid
#define ADD_VARIABLES	gid
#define BUFLEN		NSS_BUFLEN_GROUP

#include <nss/getXXbyYY_r.c>
