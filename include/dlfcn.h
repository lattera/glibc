#ifndef _DLFCN_H
#include <dlfcn/dlfcn.h>
#ifndef _ISOMAC
#include <link.h>		/* For ElfW.  */
#include <stdbool.h>

/* Internally used flag.  */
#define __RTLD_DLOPEN	0x80000000
#define __RTLD_SPROF	0x40000000
#define __RTLD_OPENEXEC	0x20000000
#define __RTLD_CALLMAP	0x10000000
#define __RTLD_AUDIT	0x08000000
#define __RTLD_SECURE	0x04000000 /* Apply additional security checks.  */
#define __RTLD_NOIFUNC	0x02000000 /* Suppress calling ifunc functions.  */

#define __LM_ID_CALLER	-2

#ifdef SHARED
/* Locally stored program arguments.  */
extern int __dlfcn_argc attribute_hidden;
extern char **__dlfcn_argv attribute_hidden;
#else
/* These variables are defined and initialized in the startup code.  */
extern int __libc_argc attribute_hidden;
extern char **__libc_argv attribute_hidden;

# define __dlfcn_argc __libc_argc
# define __dlfcn_argv __libc_argv
#endif


/* Now define the internal interfaces.  */

#define __libc_dlopen(name) \
  __libc_dlopen_mode (name, RTLD_LAZY | __RTLD_DLOPEN)
extern void *__libc_dlopen_mode  (const char *__name, int __mode);
extern void *__libc_dlsym   (void *__map, const char *__name);
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

struct link_map;

/* Close an object previously opened by _dl_open.  */
extern void _dl_close (void *map) attribute_hidden;
/* Same as above, but without locking and safety checks for user
   provided map arguments.  */
extern void _dl_close_worker (struct link_map *map, bool force)
    attribute_hidden;

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

#endif
