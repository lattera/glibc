/* Copyright (C) 1991, 93, 1995-1998, 2000, 2001 Free Software Foundation, Inc.
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


/* Old compatibility names for C types.  */
typedef unsigned char   uchar;   /* sb in libc/posix/types.h */

#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <xcoff.h>
#include <rtinit.h>
#include <dlldr.h>
#include <bits/libc-lock.h>

/* The first piece of initialized data.  */
int __data_start = 0;

#ifndef HAVE_ELF
/* Since gcc/crtstuff.c won't define it unless the ELF format is used
   we will need to define it here.  */
void *__dso_handle = NULL;
#endif

extern int errno;

/* extern __pthread_init; */

typedef void (*FPV)(void);

typedef struct crt0_info
{
   int *p_argc;
   FPV threads_init;
} INFO;


INFO    crt0_info;
int     argc;
char  **argv;
char  **__environ;
int     module_count;
caddr_t text_origin;
caddr_t data_origin;

asm("
       .toc
LL..0: .tc argc[TC],argc
LL..1: .tc argv[TC],argv
LL..2: .tc __environ[TC],__environ
LL..3: .tc module_count[TC],module_count
LL..4: .tc text_origin[TC],text_origin
LL..5: .tc data_origin[TC],data_origin
");

int main (int argc,char **argv,char **__environ);
int modinit(int argc,INFO *crt0_info, int module_count,
                     caddr_t text_origin, caddr_t data_origin);

void mod_init1(void *handler,__RTINIT *rti);

__RTINIT *find_rtinit(caddr_t text_origin,caddr_t data_origin, int module_count);

extern int *__loadx();

void __start(void)
{
#ifdef __64BIT__
asm("
      ld  17,LL..0(2)    # argc
      std 14,0(17)       # copy reg14 to argc
      ld  17,LL..1(2)    # argv
      std 15,0(17)       # copy reg15 to argv
      ld  17,LL..2(2)    # envp
      std 16,0(17)       # copy reg16 to envp
      ld  17,LL..3(2)    # module_count
      std  30,0(17)      # copy reg30 to module_count
      ld  17,LL..4(2)    # text_origin
      std  29,0(17)      # copy reg29 to text_origin
      ld  17,LL..5(2)    # data_origin
      std  28,0(17)      # copy reg28 to data_origin
");
#else
asm("
      lwz  17,LL..0(2)    # argc
      stw  3,0(17)        # copy reg3 to argc
      lwz  17,LL..1(2)    # argv
      stw  4,0(17)        # copy reg4 to argv
      lwz  17,LL..2(2)    # envp
      stw  5,0(17)        # copy reg5 to envp
      lwz  17,LL..3(2)    # module_count
      stw  30,0(17)       # copy reg30 to module_count
      lwz  17,LL..4(2)    # text_origin
      stw  29,0(17)       # copy reg29 to text_origin
      lwz  17,LL..5(2)    # data_origin
      stw  28,0(17)       # copy reg28 to data_origin
");
#endif
       crt0_info.p_argc = (int*)&argc;

/*     crt0_info.threads_init = (FPV) &__pthread_init;  */

     /*
      * Do run-time linking, if enabled and call the init()
      * for all loaded modules.
      */
      argc = modinit(argc,&crt0_info,module_count,text_origin,data_origin);

      errno=0;
     /*
      *   Call the user program.
      */
      exit (main (argc, argv, __environ));
}

/*
 *  The modinit() function performs run-time linking,
 *  if enabled, and calling the init() function for
 *  all loaded modules.
 *
 * int modinit(argc,crt0_info,module_count,text,data)
 *
 * argc         - current value of argc.
 * info         - crt0 information passed
 * module_count - number of modules loaded.
 * text         - Beginning of text address
 * data         - Beginning of data address
 */

#define DL_BUFFER_SIZE 1000

int modinit(int argc,INFO *crt0_info, int module_count,
                  caddr_t text_origin, caddr_t data_origin)
{
    int      *handler     = 0;
    __RTINIT *rtinit_info = 0;
    int flag;
    DL_INFO dl_buffer[DL_BUFFER_SIZE];
    DL_INFO *dl_info = dl_buffer;
    int i;
    FPV p;
    __libc_lock_define_initialized(static,modinit_lock);

  /*
   *   try to find __rtinit symbols
   */
   rtinit_info = find_rtinit(text_origin,data_origin,module_count);

   flag = DL_EXECQ;
   if (rtinit_info && rtinit_info->rtl) flag |= DL_LOAD_RTL;

   /*
    * get a list of modules that have __rtinit
    */
   if (__loadx(flag, dl_info, sizeof(dl_buffer))) exit(0x90);

   if (( dl_info[0].dlinfo_xflags & DL_INFO_OK))
   {
     rtinit_info = find_rtinit(dl_info[1].dlinfo_textorg,
                                  dl_info[1].dlinfo_dataorg,
                                  module_count);
     if ((rtinit_info != NULL) & (rtinit_info->rtl != NULL))
     {
        if((*rtinit_info->rtl)(dl_info,0)) exit(0x90);
     }
   }

  /*
   *    initialize threads in case any init
   *    functions need thread functions
   */
   if (crt0_info->threads_init)
     (*crt0_info->threads_init)();

   p = (FPV) __loadx(DL_GLOBALSYM | DL_SRCHLOADLIST,"pthread_init");
   if (p)
     (*p)();

   __libc_lock_lock(modinit_lock);

  /*
   *    initialization each module loaded that has __rtinit.
   */
   if (( dl_info[0].dlinfo_xflags & DL_INFO_OK))
   {
     for (i=1; i < dl_info[0].dlinfo_arraylen + 1; i++)
     {
      if (dl_info[i].dlinfo_flags & DL_HAS_RTINIT)
      {
       rtinit_info = find_rtinit(dl_info[i].dlinfo_textorg,
                                 dl_info[i].dlinfo_dataorg,
                                 module_count);
       if (rtinit_info)
       {
        mod_init1(handler,rtinit_info);
       }
      }
     }
   }

  __libc_lock_unlock(modinit_lock);
  /*
   *    reload argc if needed.
   */
  return((int) (*crt0_info->p_argc));
}

/*
 * The mod_init1 calls every initialization function
 * for a given module.
 *
 *   void mod_init1(handler, rti)
 *
 *   void *handler - if NULL init funtions for modules loaded at exec time
 *                   are being executed. Otherwise, the handler points to the
 *                   module loaded.
 *
 *   __RTINIT *rti - pointer to __rtinit data structure (with rti->init_offset
 *                   not equal to zero)
 */

void mod_init1(void *handler,__RTINIT *rtl)
{
   __RTINIT_DESCRIPTOR  *descriptor;

   descriptor =(__RTINIT_DESCRIPTOR *) ((caddr_t)&rtl->rtl + rtl->init_offset);
   while (descriptor->f)
   {
     if (!(descriptor->flags & _RT_CALLED))
     {
        descriptor->flags |=  _RT_CALLED;
        ( descriptor->f )(handler,rtl,descriptor);  /* execute init/fini */
     }
     descriptor = (__RTINIT_DESCRIPTOR *) ((caddr_t)descriptor +
                                            rtl->__rtinit_descriptor_size);
   }
}


/*
 *  Find __rtinit symbol
 *
 * __RTINIT *find_rtinit(caddr_t text_origin)
 *
 * caddr_t  text_origin - Beginning of text area
 * caddr_t  data_origin - Beginning of data area
 * int     module_count - Number of modules loaded
 * __RTINIT        *rti - pointer to __rtinit data structure
 */

__RTINIT *find_rtinit(caddr_t text_origin, caddr_t data_origin, int module_count)
{
  struct xcoffhdr *xcoff_hdr;
  SCNHDR          *sec_hdr;
  SCNHDR          *ldr_sec_hdr;
  SCNHDR          *data_sec_hdr;
  LDSYM           *ldsym_hdr;
  __RTINIT        *rtl;

  xcoff_hdr = (struct xcoffhdr *) text_origin;
  sec_hdr   = (SCNHDR *) ((caddr_t)&xcoff_hdr->aouthdr +
                                    xcoff_hdr->filehdr.f_opthdr);
  ldr_sec_hdr = (SCNHDR *) (sec_hdr + (xcoff_hdr->aouthdr.o_snloader - 1));
  ldsym_hdr   = (LDSYM  *) ((caddr_t)xcoff_hdr + ldr_sec_hdr->s_scnptr +
                                                                   LDHDRSZ);

  if ( module_count <= 0)
  {
    if ( !(ldr_sec_hdr->s_scnptr) ) return ((__RTINIT *) 0);

    if ( memcmp(ldsym_hdr,RTINIT_NAME,sizeof(RTINIT_NAME)-1))
             return ((__RTINIT *) 0);
  }

  data_sec_hdr   = (SCNHDR *) (sec_hdr + (xcoff_hdr->aouthdr.o_sndata - 1));
  rtl = (__RTINIT *) (ldsym_hdr->l_value +
                     (data_origin - data_sec_hdr->s_vaddr));
  return(rtl);
}
