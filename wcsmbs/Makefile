# Copyright (C) 1995,1996,1997,1998,1999,2000 Free Software Foundation, Inc.
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
# License along with the GNU C Library; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.

#
#	Sub-makefile for wcsmbs portion of the library.
#
subdir	:= wcsmbs

headers	:= wchar.h
distribute := wcwidth.h wcsmbsload.h

routines := wcscat wcschr wcscmp wcscpy wcscspn wcsdup wcslen wcsncat \
	    wcsncmp wcsncpy wcspbrk wcsrchr wcsspn wcstok wcsstr wmemchr \
	    wmemcmp wmemcpy wmemmove wmemset wcpcpy wcpncpy wmempcpy \
	    btowc wctob mbsinit \
	    mbrlen mbrtowc wcrtomb mbsrtowcs wcsrtombs \
	    mbsnrtowcs wcsnrtombs wcsnlen wcschrnul \
	    wcstol wcstoul wcstoll wcstoull wcstod wcstold wcstof \
	    wcstol_l wcstoul_l wcstoll_l wcstoull_l \
	    wcstod_l wcstold_l wcstof_l \
	    wcscoll wcsxfrm \
	    wcwidth wcswidth \
	    wcscoll_l wcsxfrm_l \
	    wcscasecmp wcsncase wcscasecmp_l wcsncase_l \
	    wcsmbsload

tests := tst-wcstof wcsmbs-tst1 tst-wcsnlen tst-btowc tst-mbrtowc \
	 tst-wcrtomb

include ../Rules

CFLAGS-wcwidth.c = -I../wctype
CFLAGS-wcswidth.c = -I../wctype

CFLAGS-wcstol.c = -I../stdlib
CFLAGS-wcstoul.c = -I../stdlib
CFLAGS-wcstoll.c = -I../stdlib
CFLAGS-wcstoull.c = -I../stdlib
CFLAGS-wcstod.c = -I../stdlib
CFLAGS-wcstold.c = -I../stdlib
CFLAGS-wcstof.c = -I../stdlib
CFLAGS-wcstol_l.c = -I../stdlib
CFLAGS-wcstoul_l.c = -I../stdlib
CFLAGS-wcstoll_l.c = -I../stdlib
CFLAGS-wcstoull_l.c = -I../stdlib
CFLAGS-wcstod_l.c = -I../stdlib
CFLAGS-wcstold_l.c = -I../stdlib
CFLAGS-wcstof_l.c = -I../stdlib

tst-btowc-ENV = LOCPATH=$(common-objpfx)localedata
tst-mbrtowc-ENV = LOCPATH=$(common-objpfx)localedata
tst-wcrtomb-ENV = LOCPATH=$(common-objpfx)localedata
