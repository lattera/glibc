/* Posix options supported by the GNU Hurd port of GNU libc. */ 

#define _POSIX_JOB_CONTROL	1
#define _POSIX_SAVED_IDS	1
#define _POSIX_VDISABLE		((unsigned char) -1)

/* Different Hurd filesystems might do these differently. */
#undef _POSIX_CHOWN_RESTRICTED
#undef _POSIX_NO_TRUNC



