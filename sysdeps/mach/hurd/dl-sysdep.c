/* Operating system support for run-time dynamic linker.  Hurd version.
   Copyright (C) 1995-2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* In the static library, this is all handled by dl-support.c
   or by the vanilla definitions in the rest of the C library.  */
#ifdef SHARED

#include <hurd.h>
#include <link.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <ldsodefs.h>
#include <sys/wait.h>
#include <assert.h>
#include <sysdep.h>
#include <mach/mig_support.h>
#include "hurdstartup.h"
#include <hurd/lookup.h>
#include <hurd/auth.h>
#include <hurd/term.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/uio.h>

#include <entry.h>
#include <dl-machine.h>
#include <dl-procinfo.h>

extern void __mach_init (void);

extern int _dl_argc;
extern char **_dl_argv;
extern char **_environ;

int __libc_enable_secure = 0;
INTVARDEF(__libc_enable_secure)
int __libc_multiple_libcs = 0;	/* Defining this here avoids the inclusion
				   of init-first.  */
/* This variable containts the lowest stack address ever used.  */
void *__libc_stack_end;

#if HP_TIMING_AVAIL
hp_timing_t _dl_cpuclock_offset;
#endif


struct hurd_startup_data *_dl_hurd_data;

/* This is used only within ld.so, via dl-minimal.c's __errno_location.  */
#undef errno
int errno attribute_hidden;

/* Defining these variables here avoids the inclusion of hurdsig.c.  */
unsigned long int __hurd_sigthread_stack_base;
unsigned long int __hurd_sigthread_stack_end;
unsigned long int *__hurd_sigthread_variables;

/* Defining these variables here avoids the inclusion of init-first.c.
   We need to provide temporary storage for the per-thread variables
   of the main user thread here, since it is used for storing the
   `errno' variable.  Note that this information is lost once we
   relocate the dynamic linker.  */
static unsigned long int threadvars[_HURD_THREADVAR_MAX];
unsigned long int __hurd_threadvar_stack_offset
  = (unsigned long int) &threadvars;
unsigned long int __hurd_threadvar_stack_mask;

#define FMH defined(__i386__)
#if ! FMH
# define fmh()		((void)0)
# define unfmh()	((void)0)
#else
/* XXX loser kludge for vm_map kernel bug */
#undef	ELF_MACHINE_USER_ADDRESS_MASK
#define ELF_MACHINE_USER_ADDRESS_MASK	0
static vm_address_t fmha;
static vm_size_t fmhs;
static void unfmh(void){
__vm_deallocate(__mach_task_self(),fmha,fmhs);}
static void fmh(void) {
    error_t err;int x;mach_port_t p;
    vm_address_t a=0x08000000U,max=VM_MAX_ADDRESS;
    while (!(err=__vm_region(__mach_task_self(),&a,&fmhs,&x,&x,&x,&x,&p,&x))){
      __mach_port_deallocate(__mach_task_self(),p);
      if (a+fmhs>=0x80000000U){
	max=a; break;}
      fmha=a+=fmhs;}
    if (err) assert(err==KERN_NO_SPACE);
    if (!fmha)fmhs=0;else{
    fmhs=max-fmha;
    err = __vm_map (__mach_task_self (),
		    &fmha, fmhs, 0, 0, MACH_PORT_NULL, 0, 1,
		    VM_PROT_NONE, VM_PROT_NONE, VM_INHERIT_COPY);
    assert_perror(err);}
  }
/* XXX loser kludge for vm_map kernel bug */
#endif


ElfW(Addr)
_dl_sysdep_start (void **start_argptr,
		  void (*dl_main) (const ElfW(Phdr) *phdr, ElfW(Word) phent,
				   ElfW(Addr) *user_entry,
				   ElfW(auxv_t) *auxv))
{
  void go (intptr_t *argdata)
    {
      extern unsigned int _dl_skip_args; /* rtld.c */
      char **p;

      /* Cache the information in various global variables.  */
      _dl_argc = *argdata;
      _dl_argv = 1 + (char **) argdata;
      _environ = &_dl_argv[_dl_argc + 1];
      for (p = _environ; *p++;); /* Skip environ pointers and terminator.  */

      if ((void *) p == _dl_argv[0])
	{
	  static struct hurd_startup_data nodata;
	  _dl_hurd_data = &nodata;
	  nodata.user_entry = (vm_address_t) ENTRY_POINT;
	}
      else
	_dl_hurd_data = (void *) p;

      INTUSE(__libc_enable_secure) = _dl_hurd_data->flags & EXEC_SECURE;

      if (_dl_hurd_data->flags & EXEC_STACK_ARGS &&
	  _dl_hurd_data->user_entry == 0)
	_dl_hurd_data->user_entry = (vm_address_t) ENTRY_POINT;

unfmh();			/* XXX */

#if 0				/* XXX make this work for real someday... */
      if (_dl_hurd_data->user_entry == (vm_address_t) ENTRY_POINT)
	/* We were invoked as a command, not as the program interpreter.
	   The generic ld.so code supports this: it will parse the args
	   as "ld.so PROGRAM [ARGS...]".  For booting the Hurd, we
	   support an additional special syntax:
	     ld.so [-LIBS...] PROGRAM [ARGS...]
	   Each LIBS word consists of "FILENAME=MEMOBJ";
	   for example "-/lib/libc.so=123" says that the contents of
	   /lib/libc.so are found in a memory object whose port name
	   in our task is 123.  */
	while (_dl_argc > 2 && _dl_argv[1][0] == '-' && _dl_argv[1][1] != '-')
	  {
	    char *lastslash, *memobjname, *p;
	    struct link_map *l;
	    mach_port_t memobj;
	    error_t err;

	    ++_dl_skip_args;
	    --_dl_argc;
	    p = _dl_argv++[1] + 1;

	    memobjname = strchr (p, '=');
	    if (! memobjname)
	      _dl_sysdep_fatal ("Bogus library spec: ", p, "\n", NULL);
	    *memobjname++ = '\0';
	    memobj = 0;
	    while (*memobjname != '\0')
	      memobj = (memobj * 10) + (*memobjname++ - '0');

	    /* Add a user reference on the memory object port, so we will
	       still have one after _dl_map_object_from_fd calls our
	       `close'.  */
	    err = __mach_port_mod_refs (__mach_task_self (), memobj,
					MACH_PORT_RIGHT_SEND, +1);
	    assert_perror (err);

	    lastslash = strrchr (p, '/');
	    l = _dl_map_object_from_fd (lastslash ? lastslash + 1 : p,
					memobj, strdup (p), 0);

	    /* Squirrel away the memory object port where it
	       can be retrieved by the program later.  */
	    l->l_info[DT_NULL] = (void *) memobj;
	  }
#endif

      /* Call elf/rtld.c's main program.  It will set everything
	 up and leave us to transfer control to USER_ENTRY.  */
      (*dl_main) ((const ElfW(Phdr) *) _dl_hurd_data->phdr,
		  _dl_hurd_data->phdrsz / sizeof (ElfW(Phdr)),
		  &_dl_hurd_data->user_entry, NULL);

      /* The call above might screw a few things up.

	 First of all, if _dl_skip_args is nonzero, we are ignoring
	 the first few arguments.  However, if we have no Hurd startup
	 data, it is the magical convention that ARGV[0] == P.  The
	 startup code in init-first.c will get confused if this is not
	 the case, so we must rearrange things to make it so.  We'll
	 overwrite the origional ARGV[0] at P with ARGV[_dl_skip_args].

	 Secondly, if we need to be secure, it removes some dangerous
	 environment variables.  If we have no Hurd startup date this
	 changes P (since that's the location after the terminating
	 NULL in the list of environment variables).  We do the same
	 thing as in the first case but make sure we recalculate P.
	 If we do have Hurd startup data, we have to move the data
	 such that it starts just after the terminating NULL in the
	 environment list.

	 We use memmove, since the locations might overlap.  */
      if (INTUSE(__libc_enable_secure) || _dl_skip_args)
	{
	  char **newp;

	  for (newp = _environ; *newp++;);

	  if (_dl_argv[-_dl_skip_args] == (char *) p)
	    {
	      if ((char *) newp != _dl_argv[0])
		{
		  assert ((char *) newp < _dl_argv[0]);
		  _dl_argv[0] = memmove ((char *) newp, _dl_argv[0],
					 strlen (_dl_argv[0]) + 1);
		}
	    }
	  else
	    {
	      if ((void *) newp != _dl_hurd_data)
		memmove (newp, _dl_hurd_data, sizeof (*_dl_hurd_data));
	    }
	}

      {
	extern void _dl_start_user (void);
	/* Unwind the stack to ARGDATA and simulate a return from _dl_start
	   to the RTLD_START code which will run the user's entry point.  */
	RETURN_TO (argdata, &_dl_start_user, _dl_hurd_data->user_entry);
      }
    }

  /* Set up so we can do RPCs.  */
  __mach_init ();

  /* Initialize frequently used global variable.  */
  GLRO(dl_pagesize) = __getpagesize ();

#if HP_TIMING_AVAIL
  HP_TIMING_NOW (_dl_cpuclock_offset);
#endif

fmh();				/* XXX */

  /* See hurd/hurdstartup.c; this deals with getting information
     from the exec server and slicing up the arguments.
     Then it will call `go', above.  */
  _hurd_startup (start_argptr, &go);

  LOSE;
  abort ();
}

void
internal_function
_dl_sysdep_start_cleanup (void)
{
  /* Deallocate the reply port and task port rights acquired by
     __mach_init.  We are done with them now, and the user will
     reacquire them for himself when he wants them.  */
  __mig_dealloc_reply_port (MACH_PORT_NULL);
  __mach_port_deallocate (__mach_task_self (), __mach_task_self_);
}

/* Minimal open/close/mmap implementation sufficient for initial loading of
   shared libraries.  These are weak definitions so that when the
   dynamic linker re-relocates itself to be user-visible (for -ldl),
   it will get the user's definition (i.e. usually libc's).  */

/* Open FILE_NAME and return a Hurd I/O for it in *PORT, or return an
   error.  If STAT is non-zero, stat the file into that stat buffer.  */
static error_t
open_file (const char *file_name, int flags,
	   mach_port_t *port, struct stat64 *stat)
{
  enum retry_type doretry;
  char retryname[1024];		/* XXX string_t LOSES! */
  file_t startdir;
  error_t err;

  error_t use_init_port (int which, error_t (*operate) (file_t))
    {
      return (which < _dl_hurd_data->portarraysize
	      ? ((*operate) (_dl_hurd_data->portarray[which]))
	      : EGRATUITOUS);
    }
  file_t get_dtable_port (int fd)
    {
      if ((unsigned int) fd < _dl_hurd_data->dtablesize
	  && _dl_hurd_data->dtable[fd] != MACH_PORT_NULL)
	{
	  __mach_port_mod_refs (__mach_task_self (), _dl_hurd_data->dtable[fd],
				MACH_PORT_RIGHT_SEND, +1);
	  return _dl_hurd_data->dtable[fd];
	}
      errno = EBADF;
      return MACH_PORT_NULL;
    }

  assert (!(flags & ~(O_READ | O_CLOEXEC)));

  startdir = _dl_hurd_data->portarray[file_name[0] == '/' ?
				      INIT_PORT_CRDIR : INIT_PORT_CWDIR];

  while (file_name[0] == '/')
    file_name++;

  err = __dir_lookup (startdir, (char *)file_name, O_RDONLY, 0,
		      &doretry, retryname, port);

  if (!err)
    err = __hurd_file_name_lookup_retry (use_init_port, get_dtable_port,
					 __dir_lookup, doretry, retryname,
					 O_RDONLY, 0, port);
  if (!err && stat)
    {
      err = __io_stat (*port, stat);
      if (err)
	__mach_port_deallocate (__mach_task_self (), *port);
    }

  return err;
}

int weak_function
__open (const char *file_name, int mode, ...)
{
  mach_port_t port;
  error_t err = open_file (file_name, mode, &port, 0);
  if (err)
    return __hurd_fail (err);
  else
    return (int)port;
}

int weak_function
__close (int fd)
{
  if (fd != (int) MACH_PORT_NULL)
    __mach_port_deallocate (__mach_task_self (), (mach_port_t) fd);
  return 0;
}

__ssize_t weak_function
__libc_read (int fd, void *buf, size_t nbytes)
{
  error_t err;
  char *data;
  mach_msg_type_number_t nread;

  data = buf;
  nread = nbytes;
  err = __io_read ((mach_port_t) fd, &data, &nread, -1, nbytes);
  if (err)
    return __hurd_fail (err);

  if (data != buf)
    {
      memcpy (buf, data, nread);
      __vm_deallocate (__mach_task_self (), (vm_address_t) data, nread);
    }

  return nread;
}
libc_hidden_weak (__libc_read)

__ssize_t weak_function
__libc_write (int fd, const void *buf, size_t nbytes)
{
  error_t err;
  mach_msg_type_number_t nwrote;

  assert (fd < _hurd_init_dtablesize);

  err = __io_write (_hurd_init_dtable[fd], buf, nbytes, -1, &nwrote);
  if (err)
    return __hurd_fail (err);

  return nwrote;
}
libc_hidden_weak (__libc_write)

/* This is only used for printing messages (see dl-misc.c).  */
__ssize_t weak_function
__writev (int fd, const struct iovec *iov, int niov)
{
  if (fd >= _hurd_init_dtablesize)
    {
      errno = EBADF;
      return -1;
    }

  int i;
  size_t total = 0;
  for (i = 0; i < niov; ++i)
    total += iov[i].iov_len;

  if (total != 0)
    {
      char buf[total], *bufp = buf;
      error_t err;
      mach_msg_type_number_t nwrote;

      for (i = 0; i < niov; ++i)
	bufp = (memcpy (bufp, iov[i].iov_base, iov[i].iov_len)
		+ iov[i].iov_len);

      err = __io_write (_hurd_init_dtable[fd], buf, total, -1, &nwrote);
      if (err)
	return __hurd_fail (err);

      return nwrote;
    }
  return 0;
}


off64_t weak_function
__libc_lseek64 (int fd, off64_t offset, int whence)
{
  error_t err;

  err = __io_seek ((mach_port_t) fd, offset, whence, &offset);
  if (err)
    return __hurd_fail (err);

  return offset;
}

__ptr_t weak_function
__mmap (__ptr_t addr, size_t len, int prot, int flags, int fd, off_t offset)
{
  error_t err;
  vm_prot_t vmprot;
  vm_address_t mapaddr;
  mach_port_t memobj_rd, memobj_wr;

  vmprot = VM_PROT_NONE;
  if (prot & PROT_READ)
    vmprot |= VM_PROT_READ;
  if (prot & PROT_WRITE)
    vmprot |= VM_PROT_WRITE;
  if (prot & PROT_EXEC)
    vmprot |= VM_PROT_EXECUTE;

  if (flags & MAP_ANON)
    memobj_rd = MACH_PORT_NULL;
  else
    {
      assert (!(flags & MAP_SHARED));
      err = __io_map ((mach_port_t) fd, &memobj_rd, &memobj_wr);
      if (err)
	return __hurd_fail (err), MAP_FAILED;
      __mach_port_deallocate (__mach_task_self (), memobj_wr);
    }

  mapaddr = (vm_address_t) addr;
  err = __vm_map (__mach_task_self (),
		  &mapaddr, (vm_size_t) len, ELF_MACHINE_USER_ADDRESS_MASK,
		  !(flags & MAP_FIXED),
		  memobj_rd,
		  (vm_offset_t) offset,
		  flags & (MAP_COPY|MAP_PRIVATE),
		  vmprot, VM_PROT_ALL,
		  (flags & MAP_SHARED) ? VM_INHERIT_SHARE : VM_INHERIT_COPY);
  if (err == KERN_NO_SPACE && (flags & MAP_FIXED))
    {
      /* XXX this is not atomic as it is in unix! */
      /* The region is already allocated; deallocate it first.  */
      err = __vm_deallocate (__mach_task_self (), mapaddr, len);
      if (! err)
	err = __vm_map (__mach_task_self (),
			&mapaddr, (vm_size_t) len,
			ELF_MACHINE_USER_ADDRESS_MASK,
			!(flags & MAP_FIXED),
			memobj_rd, (vm_offset_t) offset,
			flags & (MAP_COPY|MAP_PRIVATE),
			vmprot, VM_PROT_ALL,
			(flags & MAP_SHARED)
			? VM_INHERIT_SHARE : VM_INHERIT_COPY);
    }

  if ((flags & MAP_ANON) == 0)
    __mach_port_deallocate (__mach_task_self (), memobj_rd);

  if (err)
    return __hurd_fail (err), MAP_FAILED;
  return (__ptr_t) mapaddr;
}

int weak_function
__fxstat64 (int vers, int fd, struct stat64 *buf)
{
  error_t err;

  assert (vers == _STAT_VER);

  err = __io_stat ((mach_port_t) fd, buf);
  if (err)
    return __hurd_fail (err);

  return 0;
}
libc_hidden_def (__fxstat64)

int weak_function
__xstat64 (int vers, const char *file, struct stat64 *buf)
{
  error_t err;
  mach_port_t port;

  assert (vers == _STAT_VER);

  err = open_file (file, 0, &port, buf);
  if (err)
    return __hurd_fail (err);

  __mach_port_deallocate (__mach_task_self (), port);

  return 0;
}
libc_hidden_def (__xstat64)

/* This function is called by the dynamic linker (rtld.c) to check
   whether debugging malloc is allowed even for SUID binaries.  This
   stub will always fail, which means that malloc-debugging is always
   disabled for SUID binaries.  */
int weak_function
__access (const char *file, int type)
{
  errno = ENOSYS;
  return -1;
}

pid_t weak_function
__getpid ()
{
  pid_t pid, ppid;
  int orphaned;

  if (__proc_getpids (_dl_hurd_data->portarray[INIT_PORT_PROC],
		      &pid, &ppid, &orphaned))
    return -1;

  return pid;
}

/* This is called only in some strange cases trying to guess a value
   for $ORIGIN for the executable.  The dynamic linker copes with
   getcwd failing (dl-object.c), and it's too much hassle to include
   the functionality here.  (We could, it just requires duplicating or
   reusing getcwd.c's code but using our special lookup function as in
   `open', above.)  */
char *
weak_function
__getcwd (char *buf, size_t size)
{
  errno = ENOSYS;
  return NULL;
}

void weak_function attribute_hidden
_exit (int status)
{
  __proc_mark_exit (_dl_hurd_data->portarray[INIT_PORT_PROC],
		    W_EXITCODE (status, 0), 0);
  while (__task_terminate (__mach_task_self ()))
    __mach_task_self_ = (__mach_task_self) ();
}
/* We need this alias to satisfy references from libc_pic.a objects
   that were affected by the libc_hidden_proto declaration for _exit.  */
strong_alias (_exit, __GI__exit)

/* Try to get a machine dependent instruction which will make the
   program crash.  This is used in case everything else fails.  */
#include <abort-instr.h>
#ifndef ABORT_INSTRUCTION
/* No such instruction is available.  */
# define ABORT_INSTRUCTION
#endif

void weak_function
abort (void)
{
  /* Try to abort using the system specific command.  */
  ABORT_INSTRUCTION;

  /* If the abort instruction failed, exit.  */
  _exit (127);

  /* If even this fails, make sure we never return.  */
  while (1)
    /* Try for ever and ever.  */
    ABORT_INSTRUCTION;
}

/* We need this alias to satisfy references from libc_pic.a objects
   that were affected by the libc_hidden_proto declaration for abort.  */
strong_alias (abort, __GI_abort)

/* This function is called by interruptible RPC stubs.  For initial
   dynamic linking, just use the normal mach_msg.  Since this defn is
   weak, the real defn in libc.so will override it if we are linked into
   the user program (-ldl).  */

error_t weak_function
_hurd_intr_rpc_mach_msg (mach_msg_header_t *msg,
			 mach_msg_option_t option,
			 mach_msg_size_t send_size,
			 mach_msg_size_t rcv_size,
			 mach_port_t rcv_name,
			 mach_msg_timeout_t timeout,
			 mach_port_t notify)
{
  return __mach_msg (msg, option, send_size, rcv_size, rcv_name,
		     timeout, notify);
}


void
internal_function
_dl_show_auxv (void)
{
  /* There is nothing to print.  Hurd has no auxiliary vector.  */
}


void weak_function
_dl_init_first (int argc, ...)
{
  /* This no-op definition only gets used if libc is not linked in.  */
}

#endif /* SHARED */
