#ifndef _DLFCN_H
#include <dlfcn/dlfcn.h>
#include <link.h>		/* For ElfW.  */

/* Internally used flag.  */
#define __RTLD_DLOPEN	0x80000000
#define __RTLD_SPROF	0x40000000
#define __RTLD_OPENEXEC	0x20000000
#define __RTLD_CALLMAP	0x10000000

#define __LM_ID_CALLER	-2

/* Now define the internal interfaces.  */
extern void *__dlvsym (void *__handle, __const char *__name,
		       __const char *__version);

#define __libc_dlopen(name) __libc_dlopen_mode (name, RTLD_LAZY)
extern void *__libc_dlopen_mode  (__const char *__name, int __mode);
extern void *__libc_dlsym   (void *__map, __const char *__name);
extern int   __libc_dlclose (void *__map);
libc_hidden_proto (__libc_dlopen_mode)
libc_hidden_proto (__libc_dlsym)
libc_hidden_proto (__libc_dlclose)

/* Locate shared object containing the given address.  */
#ifdef ElfW
extern int _dl_addr (const void *address, Dl_info *info,
		     struct link_map **mapp, const ElfW(Sym) **symbolp)
     internal_function;
libc_hidden_proto (_dl_addr)
#endif

/* Open the shared object NAME, relocate it, and run its initializer if it
   hasn't already been run.  MODE is as for `dlopen' (see <dlfcn.h>).  If
   the object is already opened, returns its existing map.  */
extern void *_dl_open (const char *name, int mode, const void *caller,
		       Lmid_t nsid)
     internal_function;
libc_hidden_proto (_dl_open)

/* Close an object previously opened by _dl_open.  */
extern void _dl_close (void *map)
     internal_function;
libc_hidden_proto (_dl_close)

/* Look up NAME in shared object HANDLE (which may be RTLD_DEFAULT or
   RTLD_NEXT).  WHO is the calling function, for RTLD_NEXT.  Returns
   the symbol value, which may be NULL.  */
extern void *_dl_sym (void *handle, const char *name, void *who)
    internal_function;

/* Look up version VERSION of symbol NAME in shared object HANDLE
   (which may be RTLD_DEFAULT or RTLD_NEXT).  WHO is the calling
   function, for RTLD_NEXT.  Returns the symbol value, which may be
   NULL.  */
extern void *_dl_vsym (void *handle, const char *name, const char *version,
		       void *who)
    internal_function;

/* Call OPERATE, catching errors from `dl_signal_error'.  If there is no
   error, *ERRSTRING is set to null.  If there is an error, *ERRSTRING is
   set to a string constructed from the strings passed to _dl_signal_error,
   and the error code passed is the return value and *OBJNAME is set to
   the object name which experienced the problems.  ERRSTRING if nonzero
   points to a malloc'ed string which the caller has to free after use.
   ARGS is passed as argument to OPERATE.  */
extern int _dl_catch_error (const char **objname, const char **errstring,
			    void (*operate) (void *),
			    void *args)
     internal_function;

/* Helper function for <dlfcn.h> functions.  Runs the OPERATE function via
   _dl_catch_error.  Returns zero for success, nonzero for failure; and
   arranges for `dlerror' to return the error details.
   ARGS is passed as argument to OPERATE.  */
extern int _dlerror_run (void (*operate) (void *), void *args)
     internal_function;

#endif
