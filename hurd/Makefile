# Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

subdir := hurd

all:

# Some things below (but before including Rules) use configuration variables.
include ../Makeconfig


headers = hurd.h $(interface-headers) \
	  $(addprefix hurd/,fd.h id.h port.h signal.h userlink.h \
		            resource.h threadvar.h)

distribute := hurdstartup.h hurdfault.h intr-rpc.defs STATUS

# The RPC interfaces go in a separate library.
interface-library := libhurduser
user-interfaces		:= $(addprefix hurd/,\
				       auth process startup \
				       msg msg_reply msg_request \
				       exec core interrupt \
				       fs fsys io term socket ifsock)
server-interfaces	:= hurd/msg

routines = hurdstartup hurdinit \
	   hurdid hurdlookup hurdpid hurdrlimit hurdprio hurdexec \
	   setauth \
	   pid2task task2pid \
	   getuids setuids getumask fchroot \
	   hurdsock hurdauth invoke-trans \
	   privports \
	   msgportdemux \
	   fopenport \
 	   vpprintf \
	   ports-get ports-set hurdports hurdmsg \
	   $(sig) $(dtable) hurdinline port-cleanup
sig	= hurdsig hurdfault faultexc siginfo hurd-raise preempt-sig \
	  trampoline longjmp-ts catch-exc exc2signal hurdkill sigunwind \
	  thread-self thread-cancel intr-msg
dtable	= dtable port2fd new-fd alloc-fd intern-fd \
	  getdport openport \
	  fd-close fd-read fd-write hurdioctl ctty-input ctty-output

# XXX this is a temporary hack; see hurdmalloc.h
routines += hurdmalloc
distribute += hurdmalloc.h

# Get the proper definition of `hurd-srcdir'.
include ../sysdeps/mach/hurd/Makefile

# Use and install the Hurd header files directly out of the Hurd source.

# Find the MiG defs files in the Hurd source.
vpath %.defs $(hurd-srcdir)

# Install all .h and .defs files we find in the Hurd's hurd/ directory.
hurd-headers := $(patsubst $(hurd-srcdir)/%,%,\
			   $(wildcard $(addprefix $(hurd-srcdir)/hurd/,\
						  *.defs *.h)))


# Don't distribute the Hurd headers; they are in the Hurd distribution.
dont_distribute = $(hurd-headers)

# DO NOT try to remake these in any way!!!
$(addprefix $(hurd-srcdir)/,$(hurd-headers)) : ;
install-others += $(addprefix $(includedir)/,$(hurd-headers))
$(includedir)/hurd/%: $(hurd-srcdir)/hurd/%; $(do-install)

include ../mach/Machrules
include ../Rules

# intr-rpc.defs defines the INTR_INTERFACE macro to make the generated RPC
# stubs import <hurd/signal.h> and #define __mach_msg to
# _hurd_intr_rpc_mach_msg.
user-MIGFLAGS += -imacros intr-rpc.defs

$(objpfx)fault%.c $(objpfx)fault%.h: $(mach-srcdir)/mach/%.defs
	$(MIG) $(MIGFLAGS) -prefix _hurdsig_fault_ \
	       -server $(@:.h=.c) -sheader $(@:.c=.h) \
	       -user /dev/null -header /dev/null \
	       $<
generated += faultexc.c faultexc.h

# We need this static dependency to get faultexc.h generated the first time.
$(objpfx)hurdfault.o $(objpfx)hurdfault.d: \
	$(objpfx)faultexc.h $(objpfx)faultexc.c
