# Copyright (C) 1991, 92, 93, 94, 95, 96, 97, 98 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License
# as published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If not,
# write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

subdir := hurd

all:

# Some things below (but before including Rules) use configuration variables.
include ../Makeconfig


headers = hurd.h $(interface-headers) \
	  $(addprefix hurd/,fd.h id.h port.h signal.h sigpreempt.h ioctl.h\
			    userlink.h resource.h threadvar.h lookup.h)

# The RPC interfaces go in a separate library.
interface-library := libhurduser
user-interfaces		:= $(addprefix hurd/,\
				       auth startup \
				       process process_request \
				       msg msg_reply msg_request \
				       exec exec_startup crash interrupt \
				       fs fsys io term tioctl socket ifsock)
server-interfaces	:= hurd/msg faultexc

routines = hurdstartup hurdinit \
	   hurdid hurdlookup hurdpid hurdrlimit hurdprio hurdexec \
	   get-host set-host \
	   path-lookup \
	   setauth \
	   pid2task task2pid \
	   getuids setuids getumask fchroot \
	   hurdsock hurdauth \
	   privports \
	   msgportdemux \
	   fopenport \
	   vpprintf \
	   ports-get ports-set hurdports hurdmsg \
	   $(sig) $(dtable) hurdinline port-cleanup report-wait
sig	= hurdsig hurdfault siginfo hurd-raise preempt-sig \
	  trampoline longjmp-ts catch-exc exc2signal hurdkill sigunwind \
	  thread-self thread-cancel intr-msg catch-signal
dtable	= dtable port2fd new-fd alloc-fd intern-fd \
	  getdport openport \
	  fd-close fd-read fd-write hurdioctl ctty-input ctty-output
distribute = hurdstartup.h hurdfault.h hurdhost.h \
	     faultexc.defs intr-rpc.defs intr-rpc.h intr-msg.h Notes

# XXX this is a temporary hack; see hurdmalloc.h
routines += hurdmalloc
distribute += hurdmalloc.h

include ../mach/Machrules
include ../Rules

# intr-rpc.defs defines the INTR_INTERFACE macro to make the generated RPC
# stubs import <hurd/signal.h> and #define __mach_msg to
# _hurd_intr_rpc_mach_msg.
user-MIGFLAGS += -imacros intr-rpc.defs

# The special exc server for sigthread faults uses a special prefix.
MIGFLAGS-faultexc = -prefix _hurdsig_fault_

# We need this static dependency to get faultexc.h generated the first time.
$(objpfx)hurdfault.o $(objpfx)hurdfault.d: \
	$(objpfx)faultexc_server.h $(objpfx)faultexc_server.c
