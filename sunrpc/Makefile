# Copyright (C) 1994,95,96,97,98,99,2000,2001 Free Software Foundation, Inc.
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
# License along with the GNU C Library; see the file COPYING.LIB.  If not,
# write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

#
#	Sub-makefile for sunrpc portion of the library.
#
subdir	:= sunrpc

# The code in this subdirectory is taken from Sun's RPCSRC-4.0
# distribution with some additional changes from the TI-RPC package
# which is also available from Sun.  The files are heavily changed to
# compile cleanly and to fit in the GNU environment.  All the code
# from Sun's rpc, etc, and rpcgen subdirectories is in this directory;
# the rpc subdirectory contains only the header files.  Other than
# that, several files were renamed so as not to exceed 14-character
# file name limits:
#
#	authunix_prot.c -> authuxprot.c
#	bindresvport.c -> bindrsvprt.c
#	clnt_generic.c -> clnt_gen.c
#	clnt_perror.c -> clnt_perr.c
#	clnt_simple.c -> clnt_simp.c
#	get_myaddress.c -> get_myaddr.c
#	pmap_getmaps.c -> pm_getmaps.c
#	pmap_getport.c -> pm_getport.c
#	rpc_callmsg.c -> rpc_cmsg.c
#	rpc_commondata.c -> rpc_common.c
#	rpc_dtablesize.c -> rpc_dtable.c
#	svc_auth_unix.c -> svc_authux.c
#	xdr_reference.c -> xdr_ref.c

rpcsvc = bootparam_prot.x nlm_prot.x rstat.x \
	 yppasswd.x klm_prot.x rex.x sm_inter.x mount.x \
	 rusers.x spray.x nfs_prot.x rquota.x key_prot.x
headers = $(addprefix rpc/,auth.h auth_unix.h clnt.h netdb.h pmap_clnt.h \
			   pmap_prot.h pmap_rmt.h rpc.h rpc_msg.h svc.h \
			   svc_auth.h types.h xdr.h auth_des.h \
			   des_crypt.h key_prot.h rpc_des.h) \
	  $(rpcsvc:%=rpcsvc/%) rpcsvc/bootparam.h
install-others = $(inst_sysconfdir)/rpc
generated = $(rpcsvc:%.x=rpcsvc/%.h) $(rpcsvc:%.x=x%.c) $(rpcsvc:%.x=x%.stmp) \
	    $(rpcsvc:%.x=rpcsvc/%.stmp) rpc-proto.c rpc-proto.d rpcgen

routines := auth_none auth_unix authuxprot bindrsvprt \
	    clnt_gen clnt_perr clnt_raw clnt_simp clnt_tcp \
	    clnt_udp rpc_dtable get_myaddr getrpcport \
	    pmap_clnt pm_getmaps pm_getport pmap_prot \
	    pmap_prot2 pmap_rmt rpc_prot rpc_common rpc_cmsg \
	    svc svc_auth svc_authux svc_raw svc_run svc_simple \
	    svc_tcp svc_udp xdr xdr_array xdr_float xdr_mem \
	    xdr_rec xdr_ref xdr_stdio publickey xdr_sizeof \
	    auth_des authdes_prot des_crypt des_impl des_soft \
	    key_call key_prot netname openchild rtime svcauth_des xcrypt\
	    clnt_unix svc_unix create_xid xdr_intXX_t

others := rpcinfo
install-bin := rpcgen
install-sbin := rpcinfo
rpcgen-objs = rpc_main.o rpc_hout.o rpc_cout.o rpc_parse.o \
	      rpc_scan.o rpc_util.o rpc_svcout.o rpc_clntout.o \
	      rpc_tblout.o rpc_sample.o
# These headers are part of rpcgen.
distribute := proto.h rpc_util.h rpc_parse.h rpc_scan.h \
	      $(rpcgen-objs:.o=.c) etc.rpc \
	      errqueue.h
extra-objs = $(rpcgen-objs)

all: # Make this the default target; it will be defined in Rules.

include ../Makeconfig

ifeq (yes,$(build-static-nss))
otherlibs += $(nssobjdir)/libnss_files.a $(resolvobjdir)/libnss_dns.a \
	     $(resolvobjdir)/libresolv.a
endif

ifeq (no,$(cross-compiling))
# We can only build this library if we can run the rpcgen we build.
headers += $(rpcsvc:%.x=rpcsvc/%.h)
extra-libs := librpcsvc
extra-libs-others := librpcsvc # Make it in `others' pass, not `lib' pass.
librpcsvc-routines = $(rpcsvc:%.x=x%)
librpcsvc-inhibit-o = .os # Build no shared rpcsvc library.
omit-deps = $(librpcsvc-routines)
endif

CFLAGS-xbootparam_prot.c = -Wno-unused
CFLAGS-xnlm_prot.c = -Wno-unused
CFLAGS-xrstat.c = -Wno-unused
CFLAGS-xyppasswd.c = -Wno-unused
CFLAGS-xklm_prot.c = -Wno-unused
CFLAGS-xrex.c = -Wno-unused
CFLAGS-xsm_inter.c = -Wno-unused
CFLAGS-xmount.c = -Wno-unused
CFLAGS-xrusers.c = -Wno-unused
CFLAGS-xspray.c = -Wno-unused
CFLAGS-xnfs_prot.c = -Wno-unused
CFLAGS-xrquota.c = -Wno-unused
CFLAGS-xkey_prot.c = -Wno-unused

ifeq (yes,$(have_doors))
CPPFLAGS-key_call.c += -DHAVE_DOORS=1
endif

include ../Rules

$(objpfx)rpcgen: $(addprefix $(objpfx),$(rpcgen-objs)) \
  $(sort $(filter $(common-objpfx)libc%,$(link-libc))) \
  $(addprefix $(csu-objpfx),start.o) $(+preinit) $(+postinit)
	$(+link)

# Tell rpcgen where to find the C preprocessor.
rpcgen-cmd = $(built-program-cmd) -Y `$(CC) -print-file-name=cpp | \
				      sed 's|/cpp$$||'`

# Install the rpc data base file.
$(inst_sysconfdir)/rpc: etc.rpc $(+force)
	$(do-install)

# Generate the rpcsvc headers with rpcgen.
# We use a stamp file to avoid unnessary recompilation each time rpcgen is
# relinked.
$(rpcsvc:%.x=$(objpfx)rpcsvc/%.h): $(objpfx)rpcsvc/%.h: $(objpfx)rpcsvc/%.stmp
	@:
$(objpfx)rpcsvc/%.stmp: rpcsvc/%.x $(objpfx)rpcgen
	$(make-target-directory)
	-@rm -f ${@:stmp=T} $@
	$(rpcgen-cmd) -h $< -o ${@:stmp=T}
	$(move-if-change) $(@:stmp=T) $(@:stmp=h)
	touch $@

# Generate the rpcsvc XDR functions with rpcgen.
$(rpcsvc:%.x=$(objpfx)x%.c): $(objpfx)x%.c: $(objpfx)x%.stmp
	@:
$(objpfx)x%.stmp: rpcsvc/%.x $(objpfx)rpcgen
	-@rm -f ${@:stmp=T} $@
	$(rpcgen-cmd) -c $< -o ${@:stmp=T}
	$(move-if-change) $(@:stmp=T) $(@:stmp=c)
	touch $@

# The generated source files depend on the corresponding generated headers.
# Gratuitous dependency on generated .c file here just gets it mentioned to
# avoid being an intermediate file and getting removed.
define o-iterator-doit
$(rpcsvc:%.x=$(objpfx)x%$o): $(objpfx)x%$o: $(objpfx)x%.c $(objpfx)rpcsvc/%.h \
					    $(objpfx)rpc-proto.d
endef
object-suffixes-left = $(filter-out $(librpcsvc-inhibit-o),$(object-suffixes))
include $(o-iterator)

ifndef no_deps
-include $(objpfx)rpc-proto.d
endif

$(objpfx)rpc-proto.d: $(objpfx)%.d: $(objpfx)%.c
	$(+make-deps)
# Special file to generate dependencies for the RPC service objects.
# Collect all include directives from the source files.
$(objpfx)rpc-proto.c: $(rpcsvc:%=rpcsvc/%)
	$(make-target-directory)
	{ echo '#include <rpc/types.h>'; \
	  echo '#include <rpc/xdr.h>'; \
	  echo '#include <rpc/rpc.h>'; \
	  sed -n '/^%#include/s/%//p' $^; } > $@T
	mv -f $@T $@
