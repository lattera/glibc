ifeq ($(subdir),misc)
sysdep_routines += setfsgid setfsuid setresgid setresuid ioperm
sysdep_headers += sys/elf.h sys/io.h
endif

ifeq ($(subdir),signal)
sysdep_routines += rt_sigsuspend rt_sigprocmask rt_sigtimedwait	\
		   rt_sigqueueinfo rt_sigaction rt_sigpending \
		   sigrestorer
endif

ifeq ($(subdir),resource)
sysdep_routines += oldgetrlimit64
endif

ifeq ($(subdir),elf)
# extra shared linker files to link into dl-allobjs.so and libc
sysdep-dl-routines += dl-procinfo
sysdep_routines += dl-procinfo
# extra shared linker files to link only into dl-allobjs.so
sysdep-rtld-routines += dl-procinfo
endif
