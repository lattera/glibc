# Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

#
#	Sub-makefile for string portion of library.
#
subdir	:= string

headers	:= string.h strings.h memory.h endian.h bytesex.h

routines	:= strcat strchr strcmp strcoll strcpy strcspn strdup	\
		   strerror _strerror strlen strncat strncmp strncpy 	\
		   strrchr strpbrk strsignal strspn strstr strtok	\
		   strxfrm memchr memcmp memmove memset			\
		   bcopy bzero ffs stpcpy stpncpy			\
		   strcasecmp strncase					\
		   memccpy memcpy wordcopy strsep			\
		   swab strfry memfrob memmem

tests		:= tester testcopy test-ffs
distribute	:= memcopy.h


include ../Rules
