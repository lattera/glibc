# Set this flag here so that arm/nptl/Makefile will see it.
arm-using-eabi = yes

ifeq ($(subdir),csu)
# In order for unwinding to fail when it falls out of main, we need a
# cantunwind marker.  There's one in start.S.  To make sure we reach it, add
# unwind tables for __libc_start_main.
CFLAGS-libc-start.c += -fexceptions
endif

# Add a syscall function to each library that needs one.

ifeq ($(subdir),rt)
librt-sysdep_routines += libc-do-syscall
librt-shared-only-routines += libc-do-syscall
endif

ifeq ($(subdir),nptl)
libpthread-sysdep_routines += libc-do-syscall
libpthread-shared-only-routines += libc-do-syscall
endif

ifeq ($(subdir),resolv)
libanl-sysdep_routines += libc-do-syscall
libanl-shared-only-routines += libc-do-syscall
endif

ifeq ($(subdir),csu)
sysdep_routines += libc-do-syscall
endif

ifeq ($(subdir),nscd)
nscd-modules += libc-do-syscall
endif

ifeq ($(subdir),nss)
libnss_db-sysdep_routines += libc-do-syscall
libnss_db-shared-only-routines += libc-do-syscall
endif

ifeq ($(subdir),posix)
LDFLAGS-tst-rfc3484 += $(common-objpfx)csu/libc-do-syscall.o
LDFLAGS-tst-rfc3484-2 += $(common-objpfx)csu/libc-do-syscall.o
LDFLAGS-tst-rfc3484-3 += $(common-objpfx)csu/libc-do-syscall.o
endif
