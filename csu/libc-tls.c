/* Initialization code for TLS in statically linked application.
   Copyright (C) 2002-2016 Free Software Foundation, Inc.
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

#include <errno.h>
#include <ldsodefs.h>
#include <tls.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/param.h>


#ifdef SHARED
 #error makefile bug, this file is for static only
#endif

dtv_t _dl_static_dtv[2 + TLS_SLOTINFO_SURPLUS];


static struct
{
  struct dtv_slotinfo_list si;
  /* The dtv_slotinfo_list data structure does not include the actual
     information since it is defined as an array of size zero.  We define
     here the necessary entries.  Note that it is not important whether
     there is padding or not since we will always access the information
     through the 'si' element.  */
  struct dtv_slotinfo info[2 + TLS_SLOTINFO_SURPLUS];
} static_slotinfo;


/* Highest dtv index currently needed.  */
size_t _dl_tls_max_dtv_idx;
/* Flag signalling whether there are gaps in the module ID allocation.  */
bool _dl_tls_dtv_gaps;
/* Information about the dtv slots.  */
struct dtv_slotinfo_list *_dl_tls_dtv_slotinfo_list;
/* Number of modules in the static TLS block.  */
size_t _dl_tls_static_nelem;
/* Size of the static TLS block.  Giving this initialized value
   preallocates some surplus bytes in the static TLS area.  */
size_t _dl_tls_static_size = 2048;
/* Size actually allocated in the static TLS block.  */
size_t _dl_tls_static_used;
/* Alignment requirement of the static TLS block.  */
size_t _dl_tls_static_align;

/* Generation counter for the dtv.  */
size_t _dl_tls_generation;


/* Additional definitions needed by TLS initialization.  */
#ifdef TLS_INIT_HELPER
TLS_INIT_HELPER
#endif

static void
init_slotinfo (void)
{
  /* Create the slotinfo list.  */
  static_slotinfo.si.len = (((char *) (&static_slotinfo + 1)
			     - (char *) &static_slotinfo.si.slotinfo[0])
			    / sizeof static_slotinfo.si.slotinfo[0]);
  // static_slotinfo.si.next = NULL;	already zero

  /* The slotinfo list.  Will be extended by the code doing dynamic
     linking.  */
  GL(dl_tls_max_dtv_idx) = 1;
  GL(dl_tls_dtv_slotinfo_list) = &static_slotinfo.si;
}

static void
init_static_tls (size_t memsz, size_t align)
{
  /* That is the size of the TLS memory for this object.  The initialized
     value of _dl_tls_static_size is provided by dl-open.c to request some
     surplus that permits dynamic loading of modules with IE-model TLS.  */
  GL(dl_tls_static_size) = roundup (memsz + GL(dl_tls_static_size),
				    TLS_TCB_ALIGN);
#if TLS_TCB_AT_TP
  GL(dl_tls_static_size) += TLS_TCB_SIZE;
#endif
  GL(dl_tls_static_used) = memsz;
  /* The alignment requirement for the static TLS block.  */
  GL(dl_tls_static_align) = align;
  /* Number of elements in the static TLS block.  */
  GL(dl_tls_static_nelem) = GL(dl_tls_max_dtv_idx);
}

void
__libc_setup_tls (void)
{
  void *tlsblock;
  size_t memsz = 0;
  size_t filesz = 0;
  void *initimage = NULL;
  size_t align = 0;
  size_t max_align = TCB_ALIGNMENT;
  size_t tcb_offset;
  const ElfW(Phdr) *phdr;

  /* Look through the TLS segment if there is any.  */
  if (_dl_phdr != NULL)
    for (phdr = _dl_phdr; phdr < &_dl_phdr[_dl_phnum]; ++phdr)
      if (phdr->p_type == PT_TLS)
	{
	  /* Remember the values we need.  */
	  memsz = phdr->p_memsz;
	  filesz = phdr->p_filesz;
	  initimage = (void *) phdr->p_vaddr;
	  align = phdr->p_align;
	  if (phdr->p_align > max_align)
	    max_align = phdr->p_align;
	  break;
	}

  /* We have to set up the TCB block which also (possibly) contains
     'errno'.  Therefore we avoid 'malloc' which might touch 'errno'.
     Instead we use 'sbrk' which would only uses 'errno' if it fails.
     In this case we are right away out of memory and the user gets
     what she/he deserves.

     The initialized value of _dl_tls_static_size is provided by dl-open.c
     to request some surplus that permits dynamic loading of modules with
     IE-model TLS.  */
#if TLS_TCB_AT_TP
  /* Align the TCB offset to the maximum alignment, as
     _dl_allocate_tls_storage (in elf/dl-tls.c) does using __libc_memalign
     and dl_tls_static_align.  */
  tcb_offset = roundup (memsz + GL(dl_tls_static_size), max_align);
  tlsblock = __sbrk (tcb_offset + TLS_INIT_TCB_SIZE + max_align);
#elif TLS_DTV_AT_TP
  tcb_offset = roundup (TLS_INIT_TCB_SIZE, align ?: 1);
  tlsblock = __sbrk (tcb_offset + memsz + max_align
		     + TLS_PRE_TCB_SIZE + GL(dl_tls_static_size));
  tlsblock += TLS_PRE_TCB_SIZE;
#else
  /* In case a model with a different layout for the TCB and DTV
     is defined add another #elif here and in the following #ifs.  */
# error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
#endif

  /* Align the TLS block.  */
  tlsblock = (void *) (((uintptr_t) tlsblock + max_align - 1)
		       & ~(max_align - 1));

  /* Initialize the dtv.  [0] is the length, [1] the generation counter.  */
  _dl_static_dtv[0].counter = (sizeof (_dl_static_dtv) / sizeof (_dl_static_dtv[0])) - 2;
  // _dl_static_dtv[1].counter = 0;		would be needed if not already done

  struct link_map *main_map = GL(dl_ns)[LM_ID_BASE]._ns_loaded;

  /* Initialize the TLS block.  */
#if TLS_TCB_AT_TP
  _dl_static_dtv[2].pointer.val = ((char *) tlsblock + tcb_offset
			       - roundup (memsz, align ?: 1));
  main_map->l_tls_offset = roundup (memsz, align ?: 1);
#elif TLS_DTV_AT_TP
  _dl_static_dtv[2].pointer.val = (char *) tlsblock + tcb_offset;
  main_map->l_tls_offset = tcb_offset;
#else
# error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
#endif
  _dl_static_dtv[2].pointer.to_free = NULL;
  /* sbrk gives us zero'd memory, so we don't need to clear the remainder.  */
  memcpy (_dl_static_dtv[2].pointer.val, initimage, filesz);

  /* Install the pointer to the dtv.  */

  /* Initialize the thread pointer.  */
#if TLS_TCB_AT_TP
  INSTALL_DTV ((char *) tlsblock + tcb_offset, _dl_static_dtv);

  const char *lossage = TLS_INIT_TP ((char *) tlsblock + tcb_offset);
#elif TLS_DTV_AT_TP
  INSTALL_DTV (tlsblock, _dl_static_dtv);
  const char *lossage = TLS_INIT_TP (tlsblock);
#else
# error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
#endif
  if (__builtin_expect (lossage != NULL, 0))
    __libc_fatal (lossage);

  /* Update the executable's link map with enough information to make
     the TLS routines happy.  */
  main_map->l_tls_align = align;
  main_map->l_tls_blocksize = memsz;
  main_map->l_tls_initimage = initimage;
  main_map->l_tls_initimage_size = filesz;
  main_map->l_tls_modid = 1;

  init_slotinfo ();
  // static_slotinfo.si.slotinfo[1].gen = 0; already zero
  static_slotinfo.si.slotinfo[1].map = main_map;

  memsz = roundup (memsz, align ?: 1);

#if TLS_DTV_AT_TP
  memsz += tcb_offset;
#endif

  init_static_tls (memsz, MAX (TLS_TCB_ALIGN, max_align));
}
