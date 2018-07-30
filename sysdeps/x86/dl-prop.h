/* Support for GNU properties.  x86 version.
   Copyright (C) 2018 Free Software Foundation, Inc.
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

#ifndef _DL_PROP_H
#define _DL_PROP_H

#include <not-cancel.h>

extern void _dl_cet_check (struct link_map *, const char *)
    attribute_hidden;
extern void _dl_cet_open_check (struct link_map *)
    attribute_hidden;

static inline void __attribute__ ((always_inline))
_rtld_main_check (struct link_map *m, const char *program)
{
#if CET_ENABLED
  _dl_cet_check (m, program);
#endif
}

static inline void __attribute__ ((always_inline))
_dl_open_check (struct link_map *m)
{
#if CET_ENABLED
  _dl_cet_open_check (m);
#endif
}

static inline void __attribute__ ((unused))
_dl_process_cet_property_note (struct link_map *l,
			      const ElfW(Nhdr) *note,
			      const ElfW(Addr) size,
			      const ElfW(Addr) align)
{
#if CET_ENABLED
  /* The NT_GNU_PROPERTY_TYPE_0 note must be aliged to 4 bytes in
     32-bit objects and to 8 bytes in 64-bit objects.  Skip notes
     with incorrect alignment.  */
  if (align != (__ELF_NATIVE_CLASS / 8))
    return;

  const ElfW(Addr) start = (ElfW(Addr)) note;

  while ((ElfW(Addr)) (note + 1) - start < size)
    {
      /* Find the NT_GNU_PROPERTY_TYPE_0 note.  */
      if (note->n_namesz == 4
	  && note->n_type == NT_GNU_PROPERTY_TYPE_0
	  && memcmp (note + 1, "GNU", 4) == 0)
	{
	  /* Check for invalid property.  */
	  if (note->n_descsz < 8
	      || (note->n_descsz % sizeof (ElfW(Addr))) != 0)
	    break;

	  /* Start and end of property array.  */
	  unsigned char *ptr = (unsigned char *) (note + 1) + 4;
	  unsigned char *ptr_end = ptr + note->n_descsz;

	  do
	    {
	      unsigned int type = *(unsigned int *) ptr;
	      unsigned int datasz = *(unsigned int *) (ptr + 4);

	      ptr += 8;
	      if ((ptr + datasz) > ptr_end)
		break;

	      if (type == GNU_PROPERTY_X86_FEATURE_1_AND)
		{
		  /* The size of GNU_PROPERTY_X86_FEATURE_1_AND is 4
		     bytes.  When seeing GNU_PROPERTY_X86_FEATURE_1_AND,
		     we stop the search regardless if its size is correct
		     or not.  There is no point to continue if this note
		     is ill-formed.  */
		  if (datasz == 4)
		    {
		      unsigned int feature_1 = *(unsigned int *) ptr;
		      if ((feature_1 & GNU_PROPERTY_X86_FEATURE_1_IBT))
			l->l_cet |= lc_ibt;
		      if ((feature_1 & GNU_PROPERTY_X86_FEATURE_1_SHSTK))
			l->l_cet |= lc_shstk;
		    }
		  return;
		}

	      /* Check the next property item.  */
	      ptr += ALIGN_UP (datasz, sizeof (ElfW(Addr)));
	    }
	  while ((ptr_end - ptr) >= 8);
	}

      /* NB: Note sections like .note.ABI-tag and .note.gnu.build-id are
	 aligned to 4 bytes in 64-bit ELF objects.  */
      note = ((const void *) note
	      + ELF_NOTE_NEXT_OFFSET (note->n_namesz, note->n_descsz,
				      align));
    }
#endif
}

#ifdef FILEBUF_SIZE
static inline int __attribute__ ((unused))
_dl_process_pt_note (struct link_map *l, const ElfW(Phdr) *ph,
		     int fd, struct filebuf *fbp)
{
# if CET_ENABLED
  const ElfW(Nhdr) *note;
  ElfW(Nhdr) *note_malloced = NULL;
  ElfW(Addr) size = ph->p_filesz;

  if (ph->p_offset + size <= (size_t) fbp->len)
    note = (const void *) (fbp->buf + ph->p_offset);
  else
    {
      if (size < __MAX_ALLOCA_CUTOFF)
	note = alloca (size);
      else
	{
	  note_malloced = malloc (size);
	  note = note_malloced;
	}
      __lseek (fd, ph->p_offset, SEEK_SET);
      if (__read_nocancel (fd, (void *) note, size) != size)
	{
	  if (note_malloced)
	    free (note_malloced);
	  return -1;
	}
    }

  _dl_process_cet_property_note (l, note, size, ph->p_align);
  if (note_malloced)
    free (note_malloced);
# endif
  return 0;
}
#endif

static inline int __attribute__ ((unused))
_rtld_process_pt_note (struct link_map *l, const ElfW(Phdr) *ph)
{
  const ElfW(Nhdr) *note = (const void *) (ph->p_vaddr + l->l_addr);
  _dl_process_cet_property_note (l, note, ph->p_memsz, ph->p_align);
  return 0;
}

#endif /* _DL_PROP_H */
