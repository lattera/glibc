ifeq ($(subdir),misc)
sysdep_routines += ioperm iopl vm86 setfsgid setfsuid setresgid setresuid
sysdep_headers += sys/elf.h sys/perm.h sys/reg.h sys/vm86.h sys/debugreg.h sys/io.h
endif

ifeq ($(subdir),elf)
sysdep-others += lddlibc4
install-bin += lddlibc4

# extra shared linker files to link into dl-allobjs.so and libc
sysdep-dl-routines += dl-procinfo
sysdep_routines += dl-procinfo
# extra shared linker files to link only into dl-allobjs.so
sysdep-rtld-routines += dl-procinfo
endif

ifeq ($(subdir),csu)
CFLAGS-initfini.s += -DWEAK_GMON_START
endif

ifeq ($(subdir),resource)
sysdep_routines += oldgetrlimit64
endif
