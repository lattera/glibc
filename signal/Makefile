# Copyright (C) 1991-2018 Free Software Foundation, Inc.
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
#	Makefile for signal routines.
#
subdir	:= signal

include ../Makeconfig

headers := signal.h sys/signal.h \
	   bits/signum.h bits/signum-generic.h \
	   bits/sigcontext.h bits/sigaction.h \
	   bits/sigevent-consts.h bits/siginfo-consts.h \
	   bits/sigstack.h bits/sigthread.h bits/ss_flags.h \
	   bits/types/__sigset_t.h bits/types/sig_atomic_t.h \
	   bits/types/sigevent_t.h bits/types/siginfo_t.h \
	   bits/types/sigset_t.h bits/types/sigval_t.h \
	   bits/types/stack_t.h bits/types/struct_sigstack.h \
	   bits/types/__sigval_t.h

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

tests		:= tst-signal tst-sigset tst-sigsimple tst-raise tst-sigset2 \
		   tst-sigwait-eintr tst-sigaction \

include ../Rules

CFLAGS-sigpause.c += -fexceptions
CFLAGS-sigsuspend.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-sigtimedwait.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-sigwait.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-sigwaitinfo.c += -fexceptions -fasynchronous-unwind-tables

CFLAGS-sigreturn.c += $(no-stack-protector)
