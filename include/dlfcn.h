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

#define __libc_dlopen(name) \
  __libc_dlopen_mode (name, RTLD_LAZY | __RTLD_DLOPEN)
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

#ifdef SHARED
# define DL_CALLER_DECL /* Nothing */
# define DL_CALLER RETURN_ADDRESS (0)
#else
# define DL_CALLER_DECL , void *dl_caller
# define DL_CALLER dl_caller
#endif

struct dlfcn_hook
{
  void *(*dlopen) (const char *file, int mode, void *dl_caller);
  int (*dlclose) (void *handle);
  void *(*dlsym) (void *handle, const char *name, void *dl_caller);
  void *(*dlvsym) (void *handle, const char *name, const char *version,
		   void *dl_caller);
  char *(*dlerror) (void);
  int (*dladdr) (const void *address, Dl_info *info);
  int (*dladdr1) (const void *address, Dl_info *info,
		  void **extra_info, int flags);
  int (*dlinfo) (void *handle, int request, void *arg, void *dl_caller);
  void *(*dlmopen) (Lmid_t nsid, const char *file, int mode, void *dl_caller);
  void *pad[4];
};

extern struct dlfcn_hook *_dlfcn_hook;
libdl_hidden_proto (_dlfcn_hook)

extern void *__dlopen (const char *file, int mode DL_CALLER_DECL)
     attribute_hidden;
extern void *__dlmopen (Lmid_t nsid, const char *file, int mode DL_CALLER_DECL)
     attribute_hidden;
extern int __dlclose (void *handle)
     attribute_hidden;
extern void *__dlsym (void *handle, const char *name DL_CALLER_DECL)
     attribute_hidden;
extern void *__dlvsym (void *handle, const char *name, const char *version
		       DL_CALLER_DECL)
     attribute_hidden;
extern char *__dlerror (void)
     attribute_hidden;
extern int __dladdr (const void *address, Dl_info *info)
     attribute_hidden;
extern int __dladdr1 (const void *address, Dl_info *info,
		      void **extra_info, int flags)
     attribute_hidden;
extern int __dlinfo (void *handle, int request, void *arg DL_CALLER_DECL)
     attribute_hidden;

#ifndef SHARED
struct link_map;
extern void * __libc_dlsym_private (struct link_map *map, const char *name)
     attribute_hidden;
extern void __libc_register_dl_open_hook (struct link_map *map)
     attribute_hidden;
extern void __libc_register_dlfcn_hook (struct link_map *map)
     attribute_hidden;
#endif

#endif
