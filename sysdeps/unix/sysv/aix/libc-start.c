/* Initialization code run first thing by the XCOFF startup code.  AIX version.
   Copyright (C) 2001, 2002 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

/* hack to use uchar's */
typedef unsigned char uchar;
#include <xcoff.h>
#include <rtinit.h>
#include <dlldr.h>
#include <bits/libc-lock.h>

extern void __libc_init_first (int argc, char **argv, char **envp);

/* XXX disable for now
extern int __libc_multiple_libcs; */

/* XXX normally defined in generic/dl-sydep.c, hack it into existance
extern void *__libc_stack_end; */
void *__libc_stack_end;

  struct __libc_start_data_rec {
    void *stack;
    void *toc;
    int argc;
    char **argv;
    char **envp;
    char *data;
    char *text;
    unsigned mcount;
    unsigned special;
    int (*main)(int, char **, char **);
    void (*init)(void);
    void (*fini)(void);
    void (*rtld_fini)(void);
  };

extern struct __libc_start_data_rec __libc_start_data;
extern int errno;

/* The first piece of initialized data.  */
int __data_start = 0;

#ifndef HAVE_ELF
/* Since gcc/crtstuff.c won't define it unless the ELF format is used
   we will need to define it here.  */
void *__dso_handle = NULL;
#endif

/* AIX kernel function */
extern int __loadx (int flag, void *module, void *arg1, void *arg2,
		    void *arg3);
/* Needed by setenv */
char  **__environ;

/*
 * Find __rtinit symbol
 *
 * __RTINIT *find_rtinit()
 *
 * __RTINIT        *rti - pointer to __rtinit data structure
 */

static __RTINIT *
find_rtinit (void)
{
  struct xcoffhdr *xcoff_hdr;
  SCNHDR *sec_hdr;
  SCNHDR *ldr_sec_hdr;
  SCNHDR *data_sec_hdr;
  LDSYM *ldsym_hdr;
  __RTINIT *rtl;

  xcoff_hdr = (struct xcoffhdr *) __libc_start_data.text;
  sec_hdr   = (SCNHDR *) ((caddr_t) &xcoff_hdr->aouthdr
			  + xcoff_hdr->filehdr.f_opthdr);
  ldr_sec_hdr = (SCNHDR *) (sec_hdr + (xcoff_hdr->aouthdr.o_snloader - 1));
  ldsym_hdr   = (LDSYM  *) ((caddr_t) xcoff_hdr + ldr_sec_hdr->s_scnptr
			    + LDHDRSZ);

  if (__libc_start_data.mcount <= 0)
    {
      if (!ldr_sec_hdr->s_scnptr)
	return NULL;

      if (memcmp (ldsym_hdr, RTINIT_NAME, sizeof(RTINIT_NAME) - 1) != 0)
	return NULL;
    }

  data_sec_hdr   = (SCNHDR *) (sec_hdr + (xcoff_hdr->aouthdr.o_sndata - 1));
  rtl = (__RTINIT *) (ldsym_hdr->l_value
		      + (__libc_start_data.data - data_sec_hdr->s_vaddr));
  return rtl;
}

/* The mod_init1 calls every initialization function
   for a given module.

     void mod_init1(handler, rti)

     void *handler - if NULL init funtions for modules loaded at exec time
                     are being executed. Otherwise, the handler points to the
                     module loaded.

     __RTINIT *rti - pointer to __rtinit data structure (with rti->init_offset
                     not equal to zero)
 */

static void
mod_init1 (void *handler,__RTINIT *rtl)
{
  __RTINIT_DESCRIPTOR *descriptor;

  descriptor = (__RTINIT_DESCRIPTOR *) ((caddr_t) &rtl->rtl
					+ rtl->init_offset);
  while (descriptor->f != NULL)
    {
      if (!(descriptor->flags & _RT_CALLED))
	{
	  descriptor->flags |=  _RT_CALLED;
	  /* Execute init/fini.  */
	  descriptor->f (handler, rtl, descriptor);
	}
      descriptor = (__RTINIT_DESCRIPTOR *) ((caddr_t) descriptor
					    + rtl->__rtinit_descriptor_size);
    }
}

/* The modinit() function performs run-time linking, if enabled, and calling
   the init() function for all loaded modules.

   int modinit()
 */

#define DL_BUFFER_SIZE 1000

static int
modinit (void)
{
  int *handler = NULL;
  __RTINIT *rtinit_info = NULL;
  int flag;
  DL_INFO dl_buffer[DL_BUFFER_SIZE];
  DL_INFO *dl_info = dl_buffer;
  int i;

  /* Find __rtinit symbols */
  rtinit_info = find_rtinit ();

  flag = DL_EXECQ;
  if (rtinit_info && rtinit_info->rtl)
    flag |= DL_LOAD_RTL;

  /* Get a list of modules that have __rtinit.  */
  if (__loadx (flag, dl_info, (void *) sizeof (dl_buffer), NULL, NULL))
    exit (0x90);

  if (( dl_info[0].dlinfo_xflags & DL_INFO_OK))
    {
      rtinit_info = find_rtinit ();
      if ((rtinit_info != NULL) & (rtinit_info->rtl != NULL))
	{
	  if ((*rtinit_info->rtl) (dl_info, 0))
	    exit (0x90);
	}
    }

  /* Initialization each module loaded that has __rtinit. */
  if (dl_info[0].dlinfo_xflags & DL_INFO_OK)
    {
      for (i = 1; i < dl_info[0].dlinfo_arraylen + 1; ++i)
	if (dl_info[i].dlinfo_flags & DL_HAS_RTINIT)
	  {
	    rtinit_info = find_rtinit ();
	    if (rtinit_info)
	      mod_init1 (handler, rtinit_info);
	  }
    }

  return 0;
}


void
__libc_start_init (void)
{
  /* Do run-time linking, if enabled and call the init()
     for all loaded modules. */
  if (__libc_start_data.mcount != __libc_start_data.special)
    modinit ();
}

/* For now these are just stubs. */
void
__libc_start_fini (void)
{
}

void
__libc_start_rtld_fini (void)
{
}

void
__libc_start_main (void)
{
#ifndef SHARED

  /* The next variable is only here to work around a bug in gcc <= 2.7.2.2.
     If the address would be taken inside the expression the optimizer
     would try to be too smart and throws it away.  Grrr.  */

  /* XXX disable for now
  int *dummy_addr = &_dl_starting_up;

  __libc_multiple_libcs = dummy_addr && !_dl_starting_up; */
#endif

  /* Store the lowest stack address.  */
  __libc_stack_end = __libc_start_data.stack;

  /* Used by setenv */
  __environ = __libc_start_data.envp;

#ifndef SHARED
  /* Clear errno. */
    errno = 0;

  /* Some security at this point.  Prevent starting a SUID binary where
     the standard file descriptors are not opened.  We have to do this
     only for statically linked applications since otherwise the dynamic
     loader did the work already.  */
  if (__builtin_expect (__libc_enable_secure, 0))
    __libc_check_standard_fds ();

#endif

  /* Register the destructor of the dynamic linker if there is any.  */
  if (__builtin_expect (__libc_start_data.rtld_fini != NULL, 1))
    __cxa_atexit ((void (*) (void *)) __libc_start_data.rtld_fini, NULL, NULL);

  /* Call the initializer of the libc.  This is only needed here if we
     are compiling for the static library in which case we haven't
     run the constructors in `_dl_start_user'.  */
#ifndef SHARED
  __libc_init_first (__libc_start_data.argc, __libc_start_data.argv,
		     __libc_start_data.envp);
#endif

  /* Register the destructor of the program, if any.  */
  if (__libc_start_data.fini)
    __cxa_atexit ((void (*) (void *)) __libc_start_data.fini, NULL, NULL);

  /* Call the initializer of the program, if any.  */
#ifdef SHARED
  if (__builtin_expect (GL(dl_debug_mask) & DL_DEBUG_IMPCALLS, 0))
    _dl_debug_printf ("\ninitialize program: %s\n\n",
		      __libc_start_data.argv[0]);
#endif
  if (__libc_start_data.init)
    (*__libc_start_data.init) ();

#ifdef SHARED
  if (__builtin_expect (GL(dl_debug_mask) & DL_DEBUG_IMPCALLS, 0))
    _dl_debug_printf ("\ntransferring control: %s\n\n",
		      __libc_start_data.argv[0]);
#endif

  exit ((*__libc_start_data.main) (__libc_start_data.argc,
				   __libc_start_data.argv,
				   __libc_start_data.envp));
}
