# Copyright (C) 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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
#	Sub-makefile for I/O portion of the library.
#
subdir	:= io

headers := sys/stat.h statbuf.h \
	   fcntl.h sys/fcntl.h fcntlbits.h \
	   poll.h sys/poll.h \
	   utime.h ftw.h fts.h

routines :=							      \
	utime							      \
	mkfifo							      \
	stat fstat lstat					      \
	umask chmod fchmod mkdir				      \
	open close read write lseek access euidaccess		      \
	fcntl flock lockf					      \
	dup dup2 pipe						      \
	creat							      \
	chdir fchdir						      \
	getcwd getwd getdirname					      \
	chown fchown						      \
	ttyname isatty						      \
	link symlink readlink					      \
	unlink rmdir						      \
	ftw fts poll

others		:= pwd
tests		:= test-utime

include ../Rules
