# The default ABI is 32.
default-abi := 32

# %ebp is used to pass the 6th argument to system calls, so these
# system calls are incompatible with a frame pointer.
uses-6-syscall-arguments = -fomit-frame-pointer

ifeq ($(subdir),misc)
sysdep_routines += ioperm iopl vm86
CFLAGS-epoll_pwait.o += $(uses-6-syscall-arguments)
CFLAGS-epoll_pwait.os += $(uses-6-syscall-arguments)
CFLAGS-mmap.o += $(uses-6-syscall-arguments)
CFLAGS-mmap.os += $(uses-6-syscall-arguments)
CFLAGS-mmap64.o += $(uses-6-syscall-arguments)
CFLAGS-mmap64.os += $(uses-6-syscall-arguments)
CFLAGS-pselect.o += $(uses-6-syscall-arguments)
CFLAGS-pselect.os += $(uses-6-syscall-arguments)
CFLAGS-rtld-mmap.os += $(uses-6-syscall-arguments)
endif

ifeq ($(subdir),sysvipc)
CFLAGS-semtimedop.o += $(uses-6-syscall-arguments)
CFLAGS-semtimedop.os += $(uses-6-syscall-arguments)
endif

ifeq ($(subdir),elf)
sysdep-others += lddlibc4
install-bin += lddlibc4
endif

ifeq ($(subdir),io)
sysdep_routines += libc-do-syscall
CFLAGS-posix_fadvise64.o += $(uses-6-syscall-arguments)
CFLAGS-posix_fadvise64.os += $(uses-6-syscall-arguments)
CFLAGS-posix_fallocate.o += $(uses-6-syscall-arguments)
CFLAGS-posix_fallocate.os += $(uses-6-syscall-arguments)
CFLAGS-posix_fallocate64.o += $(uses-6-syscall-arguments)
CFLAGS-posix_fallocate64.os += $(uses-6-syscall-arguments)
CFLAGS-sync_file_range.o += $(uses-6-syscall-arguments)
CFLAGS-sync_file_range.os += $(uses-6-syscall-arguments)
CFLAGS-fallocate.o += $(uses-6-syscall-arguments)
CFLAGS-fallocate.os += $(uses-6-syscall-arguments)
CFLAGS-fallocate64.o += $(uses-6-syscall-arguments)
CFLAGS-fallocate64.os += $(uses-6-syscall-arguments)
endif

ifeq ($(subdir),nptl)
libpthread-sysdep_routines += libc-do-syscall
libpthread-shared-only-routines += libc-do-syscall
endif

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif

ifeq ($(subdir),csu)
sysdep-dl-routines += sysdep
endif

ifeq ($(subdir),nptl)
# pull in __syscall_error routine
libpthread-routines += sysdep
libpthread-shared-only-routines += sysdep
CFLAGS-pthread_rwlock_timedrdlock.o += $(uses-6-syscall-arguments)
CFLAGS-pthread_rwlock_timedrdlock.os += $(uses-6-syscall-arguments)
CFLAGS-pthread_rwlock_timedwrlock.o += $(uses-6-syscall-arguments)
CFLAGS-pthread_rwlock_timedwrlock.os += $(uses-6-syscall-arguments)
CFLAGS-sem_wait.o += $(uses-6-syscall-arguments)
CFLAGS-sem_wait.os += $(uses-6-syscall-arguments)
CFLAGS-sem_timedwait.o += $(uses-6-syscall-arguments)
CFLAGS-sem_timedwait.os += $(uses-6-syscall-arguments)
endif

ifeq ($(subdir),rt)
# pull in __syscall_error routine
librt-routines += sysdep
librt-shared-only-routines += sysdep
endif
