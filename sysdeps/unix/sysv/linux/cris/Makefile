ifeq ($(subdir),misc)
sysdep_routines += setfsgid setfsuid setresgid setresuid
endif

ifeq ($(subdir),signal)
sysdep_routines += rt_sigsuspend rt_sigprocmask rt_sigtimedwait	\
		   rt_sigqueueinfo rt_sigaction rt_sigpending
endif
