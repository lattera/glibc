# XXX For now always link against the syscalls export file.
# This is a hack until the import/export stuff is worked out.
+postctor += /lib/syscalls.exp

ifeq ($(subdir),misc)
sysdep_routines  += dl-error dl-support dl-libc dl-open dl-sym \
		    dl-close dl-addr uitrunc

static-only-routines = gprsave0 gprrest0 gprsave1 gprrest1 fprsave fprrest
endif

ifeq ($(subdir),login)
sysdep_routines += setutxent getutxent endutxent getutxid getutxline \
                   pututxline utmpxname

#sysdep_headers += utmpx.h bits/utmpx.h
#sysdep_headers += bits/utmp.h bits/utmpx.h
endif

# Don't compile the ctype glue code, since there is no old non-GNU C library.
inhibit-glue = yes

# XXX Don't know yet why this is needed in the moment.
ifeq ($(subdir),timezone)
CPPFLAGS-zic.c = -Dunix
endif
