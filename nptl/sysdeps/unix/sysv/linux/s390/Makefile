ifeq ($(enable-lock-elision),yes)
libpthread-sysdep_routines += elision-lock elision-unlock elision-timed \
			      elision-trylock

elision-CFLAGS = -mhtm
CFLAGS-elision-lock.c = $(elision-CFLAGS)
CFLAGS-elision-timed.c = $(elision-CFLAGS)
CFLAGS-elision-trylock.c = $(elision-CFLAGS)
CFLAGS-elision-unlock.c = $(elision-CFLAGS)
endif
