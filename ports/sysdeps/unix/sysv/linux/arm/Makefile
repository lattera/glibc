ifeq ($(subdir),csu)
# In order for unwinding to fail when it falls out of main, we need a
# cantunwind marker.  There's one in start.S.  To make sure we reach it, add
# unwind tables for __libc_start_main.
CFLAGS-libc-start.c += -fexceptions

sysdep_routines += aeabi_read_tp libc-aeabi_read_tp
static-only-routines += aeabi_read_tp
shared-only-routines += libc-aeabi_read_tp
endif

ifeq ($(subdir),elf)
sysdep-rtld-routines += aeabi_read_tp
endif

ifeq ($(subdir),misc)
sysdep_routines += ioperm
sysdep_headers += sys/elf.h sys/io.h
endif

ifeq ($(subdir),signal)
sysdep_routines += sigrestorer
endif

ifeq ($(subdir),resource)
sysdep_routines += oldgetrlimit64
endif

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif

# Add a syscall function to each library that needs one.

ifeq ($(subdir),crypt)
libcrypt-sysdep_routines += libc-do-syscall
endif

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

# Get value of default-abi.
include $(common-objpfx)default-abi.make

abi-variants := soft hard

ifeq (,$(filter $(default-abi),$(abi-variants)))
Unknown ABI, must be one of $(abi-variants)
endif

abi-includes :=
abi-soft-options := -U__ARM_PCS_VFP
abi-soft-condition := !defined __ARM_PCS_VFP
abi-soft-ld-soname := ld-linux.so.3
abi-hard-options := -D__ARM_PCS_VFP
abi-hard-condition := defined __ARM_PCS_VFP
abi-hard-ld-soname := ld-linux-armhf.so.3
