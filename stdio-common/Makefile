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
#	Specific makefile for stdio-common.
#
subdir	:= stdio-common

headers	:= stdio_lim.h printf.h

routines	:=							      \
	ctermid cuserid							      \
	vfprintf vprintf printf_fp reg-printf printf-prs _itoa		      \
	vsnprintf vsprintf vasprintf	      				      \
	fprintf printf snprintf sprintf asprintf			      \
	dprintf vdprintf						      \
	vfscanf vscanf vsscanf						      \
	fscanf scanf sscanf						      \
	perror psignal							      \
	tmpfile tmpnam tempnam tempname					      \
	getline getw putw						      \
	remove rename
aux	:= errlist siglist
distribute := _itoa.h printf-parse.h

tests := tst-printf tstscanf test_rdwr test-popen tstgetln test-fseek \
	 temptest tst-fileno test-fwrite \
	 xbug errnobug \
	 bug1 bug2 bug3 bug4 bug5 bug6 bug7


include ../Rules
