# Copyright (C) 1991,1992,1993,1994,1995,1996,1997,1998,2003
#	Free Software Foundation, Inc.
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
#	Makefile for signal routines.
#
subdir	:= signal

headers	:= signal.h sys/signal.h bits/signum.h bits/sigcontext.h \
	   bits/sigaction.h bits/sigset.h bits/siginfo.h bits/sigstack.h \
	   bits/sigthread.h

routines	:= signal raise killpg \
		   sigaction sigprocmask kill \
		   sigpending sigsuspend sigwait \
		   sigblock sigsetmask sigpause sigvec \
		   sigstack sigaltstack sigintr \
		   sigsetops sigempty sigfillset sigaddset sigdelset sigismem \
		   sigreturn \
		   siggetmask sysv_signal \
		   sigisempty sigandset sigorset \
		   allocrtsig sigtimedwait sigwaitinfo sigqueue \
		   sighold sigrelse sigignore sigset

tests		:= tst-signal tst-sigset tst-sigsimple tst-raise

distribute	:= sigsetops.h testrtsig.h sigset-cvt-mask.h


include ../Rules

CFLAGS-sigpause.c = -fexceptions
CFLAGS-sigsuspend.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-sigtimedwait.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-sigwait.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-sigwaitinfo.c = -fexceptions -fasynchronous-unwind-tables
