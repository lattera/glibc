# Copyright (C) 1991-2015 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

#
#	Sub-makefile for string portion of library.
#
subdir	:= string

include ../Makeconfig

headers	:= string.h strings.h memory.h endian.h bits/endian.h \
	   argz.h envz.h byteswap.h bits/byteswap.h bits/byteswap-16.h \
	   bits/string.h bits/string2.h bits/string3.h

routines	:= strcat strchr strcmp strcoll strcpy strcspn		\
		   strverscmp strdup strndup				\
		   strerror _strerror strlen strnlen			\
		   strncat strncmp strncpy				\
		   strrchr strpbrk strsignal strspn strstr strtok	\
		   strtok_r strxfrm memchr memcmp memmove memset	\
		   mempcpy bcopy bzero ffs ffsll stpcpy stpncpy		\
		   strcasecmp strncase strcasecmp_l strncase_l		\
		   memccpy memcpy wordcopy strsep strcasestr		\
		   swab strfry memfrob memmem rawmemchr strchrnul	\
		   $(addprefix argz-,append count create ctsep next	\
				     delete extract insert stringify	\
				     addsep replace)			\
		   envz basename					\
		   strcoll_l strxfrm_l string-inlines memrchr		\
		   xpg-strerror strerror_l

strop-tests	:= memchr memcmp memcpy memmove mempcpy memset memccpy	\
		   stpcpy stpncpy strcat strchr strcmp strcpy strcspn	\
		   strlen strncmp strncpy strpbrk strrchr strspn memmem	\
		   strstr strcasestr strnlen strcasecmp strncasecmp	\
		   strncat rawmemchr strchrnul bcopy bzero memrchr
tests		:= tester inl-tester noinl-tester testcopy test-ffs	\
		   tst-strlen stratcliff tst-svc tst-inlcall		\
		   bug-strncat1 bug-strspn1 bug-strpbrk1 tst-bswap	\
		   tst-strtok tst-strxfrm bug-strcoll1 tst-strfry	\
		   bug-strtok1 $(addprefix test-,$(strop-tests))	\
		   bug-envz1 tst-strxfrm2 tst-endian tst-svc2		\
		   tst-strtok_r

xtests = tst-strcoll-overflow

ifeq ($(run-built-tests),yes)
tests-special += $(objpfx)tst-svc-cmp.out
endif

include ../Rules

CFLAGS-inl-tester.c = -fno-builtin
CFLAGS-noinl-tester.c = -fno-builtin
CFLAGS-tst-strlen.c = -fno-builtin
CFLAGS-stratcliff.c = -fno-builtin
CFLAGS-test-ffs.c = -fno-builtin
CFLAGS-tst-inlcall.c = -fno-builtin

ifeq ($(run-built-tests),yes)
$(objpfx)tst-svc-cmp.out: tst-svc.expect $(objpfx)tst-svc.out
	cmp $^ > $@; \
	$(evaluate-test)
endif
