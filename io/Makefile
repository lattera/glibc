# Copyright (C) 1992-2002, 2003 Free Software Foundation, Inc.
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
#	Sub-makefile for I/O portion of the library.
#
subdir	:= io

headers := sys/stat.h bits/stat.h sys/statfs.h bits/statfs.h sys/vfs.h \
	   sys/statvfs.h bits/statvfs.h fcntl.h sys/fcntl.h bits/fcntl.h \
	   poll.h sys/poll.h bits/poll.h \
	   utime.h ftw.h fts.h sys/sendfile.h

routines :=							      \
	utime							      \
	mkfifo							      \
	stat fstat lstat mknod stat64 fstat64 lstat64		      \
	xstat fxstat lxstat xmknod xstat64 fxstat64 lxstat64	      \
	statfs fstatfs statfs64 fstatfs64			      \
	statvfs fstatvfs statvfs64 fstatvfs64			      \
	umask chmod fchmod lchmod mkdir				      \
	open open64 close read write lseek lseek64 access euidaccess  \
	fcntl flock lockf lockf64				      \
	dup dup2 pipe						      \
	creat creat64						      \
	chdir fchdir						      \
	getcwd getwd getdirname					      \
	chown fchown lchown					      \
	ttyname ttyname_r isatty				      \
	link symlink readlink					      \
	unlink rmdir						      \
	ftw ftw64 fts poll					      \
	posix_fadvise posix_fadvise64				      \
	posix_fallocate posix_fallocate64			      \
	sendfile sendfile64

# These routines will be omitted from the libc shared object.
# Instead the static object files will be included in a special archive
# linked against when the shared library will be used.
static-only-routines = stat fstat lstat mknod stat64 fstat64 lstat64

others		:= pwd
test-srcs	:= ftwtest
tests		:= test-utime test-stat test-stat2 test-lfs tst-getcwd \
		   tst-fcntl bug-ftw1 bug-ftw2 bug-ftw3 bug-ftw4 tst-statvfs

distribute	:= ftwtest-sh

include ../Rules

CFLAGS-fcntl.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-poll.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-lockf.c = -fexceptions
CFLAGS-statfs.c = -fexceptions
CFLAGS-fstatfs.c = -fexceptions
CFLAGS-statvfs.c = -fexceptions
CFLAGS-fstatvfs.c = -fexceptions
CFLAGS-fts.c = -Wno-uninitialized $(uses-callbacks) -fexceptions
CFLAGS-ftw.c = $(uses-callbacks) -fexceptions
CFLAGS-ftw64.c = $(uses-callbacks) -fexceptions
CFLAGS-lockf.c = -fexceptions
CFLAGS-posix_fallocate.c = -fexceptions
CFLAGS-posix_fallocate64.c = -fexceptions

CFLAGS-test-stat.c = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
CFLAGS-test-lfs.c = -D_LARGEFILE64_SOURCE

ifeq (yes,$(have-protected))
CFLAGS-stat.c = -DHAVE_DOT_HIDDEN
CFLAGS-fstat.c = -DHAVE_DOT_HIDDEN
CFLAGS-lstat.c = -DHAVE_DOT_HIDDEN
CFLAGS-mknod.c = -DHAVE_DOT_HIDDEN
CFLAGS-stat64.c = -DHAVE_DOT_HIDDEN
CFLAGS-fstat64.c = -DHAVE_DOT_HIDDEN
CFLAGS-lstat64.c = -DHAVE_DOT_HIDDEN
endif

test-stat2-ARGS = Makefile . $(objpfx)test-stat2

tst-statvfs-ARGS = $(objpfx)tst-statvfs tst-statvfs.c /tmp

ifeq ($(cross-compiling),no)
tests: $(objpfx)ftwtest.out

$(objpfx)ftwtest.out: $(objpfx)ftwtest
	$(SHELL) -e ftwtest-sh $(shell cd $(common-objpfx). && pwd)/ \
		 $(shell cd $(<D) && pwd)/$(<F) > $@
endif
