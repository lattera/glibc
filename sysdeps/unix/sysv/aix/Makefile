# XXX For now always link against the syscalls export file.
# This is a hack until the import/export stuff is worked out.
+postctor += /lib/syscalls.exp

# XXX This is a hack until we have the possibility to generate out own crt0.
static-start-installed-name = /usr/lib/crt0.o

ifeq ($(subdir),misc)
sysdep_routines += dl-open dl-sym dl-close restf savef uitrunc
endif

ifeq ($(subdir),login)
sysdep_routines += setutxent getutxent endutxent getutxid getutxline \
                   pututxline utmpxname

sysdep_headers += utmpx.h bits/utmpx.h
endif

# Don't compile the ctype glue code, since there is no old non-GNU C library.
inhibit-glue = yes

# XXX Don't know yet why this is needed in the moment.
ifeq ($(subdir),timezone)
CPPFLAGS-zic.c = -Dunix
endif
