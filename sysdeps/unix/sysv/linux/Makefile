ifeq ($(subdir),csu)
CFLAGS-init-first.c = -fkeep-inline-functions

sysdep_routines += errno-loc
endif

ifeq ($(subdir),db2)
CPPFLAGS += -DHAVE_LLSEEK=1
endif

ifeq ($(subdir),misc)
sysdep_routines += sysctl clone llseek getresuid getresgid umount umount2

sysdep_headers += sys/mount.h sys/acct.h sys/sysctl.h sys/mtio.h \
		  sys/io.h sys/klog.h sys/kdaemon.h \
		  sys/user.h sys/sysmacros.h sys/procfs.h sys/prctl.h \
		  sys/kd.h sys/soundcard.h sys/vt.h \
		  sys/quota.h sys/fsuid.h \
		  scsi/sg.h scsi/scsi.h sys/pci.h sys/ultrasound.h \
		  sys/sendfile.h

install-others += $(inst_includedir)/bits/syscall.h

# Generate the list of SYS_* macros for the system calls (__NR_* macros).
$(objpfx)syscall-%.h $(objpfx)syscall-%.d: ../sysdeps/unix/sysv/linux/sys/syscall.h
	rm -f $(@:.h=.d)
	{ \
	 echo '/* Generated at libc build time from kernel syscall list.  */';\
	 echo ''; \
	 echo '#ifndef _SYSCALL_H'; \
	 echo '# error "Never use <bits/syscall.h> directly; include <sys/syscall.h> instead."'; \
	 echo '#endif'; \
	 echo ''; \
	 SUNPRO_DEPENDENCIES='$(@:.h=.d) $(patsubst $(objpfx)%,$$(objpfx)%,\
						    $(@:.d=.h) $(@:.h=.d))' \
	 $(CC) -E -x c $(sysincludes) $< -D_LIBC -dM | \
	 sed -n 's@^#define __NR_\([^ ]*\) .*$$@#define SYS_\1 __NR_\1@p'; \
	} > $(@:.d=.h).new
	mv -f $(@:.d=.h).new $(@:.d=.h)

$(inst_includedir)/bits/syscall.h: $(objpfx)syscall-list.h $(+force)
	$(make-target-directory)
	if test -r $@ && cmp -s $< $@; \
        then echo 'bits/syscall.h unchanged'; \
        else $(INSTALL_DATA) $< $@; fi

ifndef no_deps
# Get the generated list of dependencies (probably /usr/include/asm/unistd.h).
-include $(objpfx)syscall-list.d
endif
generated += syscall-list.h syscall-list.d
endif

ifeq ($(subdir),time)
sysdep_headers += sys/timex.h
endif

ifeq ($(subdir),socket)
sysdep_headers += net/if.h net/if_ppp.h net/ppp-comp.h \
		  net/ppp_defs.h net/if_arp.h net/route.h net/ethernet.h \
		  net/if_slip.h net/if_packet.h net/if_shaper.h
sysdep_routines += cmsg_nxthdr sa_len
endif

ifeq ($(subdir),sunrpc)
sysdep_headers += nfs/nfs.h
endif

ifeq ($(subdir),termios)
sysdep_headers += termio.h
endif

ifeq ($(subdir),stdio-common)
# Just disable the auto generation in sysdeps/generic
inhibit-siglist := yes
endif

ifeq ($(subdir),posix)
sysdep_headers += bits/pthreadtypes.h
endif

ifeq ($(subdir),inet)
sysdep_headers += sys/socketvar.h netinet/in_systm.h \
		  netinet/if_fddi.h netinet/if_tr.h netinet/igmp.h \
		  netipx/ipx.h netash/ash.h netax25/ax25.h netatalk/at.h \
		  netrom/netrom.h netpacket/packet.h netrose/rose.h \
		  neteconet/ec.h
endif

# Don't compile the ctype glue code, since there is no old non-GNU C library.
inhibit-glue = yes

ifeq ($(subdir),dirent)
sysdep_routines += getdents64
endif

ifeq ($(subdir),nis)
CFLAGS-ypclnt.c = -DUSE_BINDINGDIR=1
endif
