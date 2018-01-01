/* Copyright (C) 1999-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 1999 and
		  Jakub Jelinek <jakub@redhat.com>, 1999.

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

/* This code is a heavily simplified version of the readelf program
   that's part of the current binutils development version.  For architectures
   which need to handle both 32bit and 64bit ELF libraries,  this file is
   included twice for each arch size.  */

/* check_ptr checks that a pointer is in the mmaped file and doesn't
   point outside it.  */
#undef check_ptr
#define check_ptr(ptr)						\
do								\
  {								\
    if ((void *)(ptr) < file_contents				\
	|| (void *)(ptr) > (file_contents+file_length))		\
      {								\
	error (0, 0, _("file %s is truncated\n"), file_name);	\
	return 1;						\
      }								\
  }								\
 while (0);

/* Returns 0 if everything is ok, != 0 in case of error.  */
int
process_elf_file (const char *file_name, const char *lib, int *flag,
		  unsigned int *osversion, char **soname, void *file_contents,
		  size_t file_length)
{
  int i;
  unsigned int j;
  ElfW(Addr) loadaddr;
  unsigned int dynamic_addr;
  size_t dynamic_size;
  char *program_interpreter;

  ElfW(Ehdr) *elf_header;
  ElfW(Phdr) *elf_pheader, *segment;
  ElfW(Dyn) *dynamic_segment, *dyn_entry;
  char *dynamic_strings;

  elf_header = (ElfW(Ehdr) *) file_contents;
  *osversion = 0;

  if (elf_header->e_ident [EI_CLASS] != ElfW (CLASS))
    {
      if (opt_verbose)
	{
	  if (elf_header->e_ident [EI_CLASS] == ELFCLASS32)
	    error (0, 0, _("%s is a 32 bit ELF file.\n"), file_name);
	  else if (elf_header->e_ident [EI_CLASS] == ELFCLASS64)
	    error (0, 0, _("%s is a 64 bit ELF file.\n"), file_name);
	  else
	    error (0, 0, _("Unknown ELFCLASS in file %s.\n"), file_name);
	}
      return 1;
    }

  if (elf_header->e_type != ET_DYN)
    {
      error (0, 0, _("%s is not a shared object file (Type: %d).\n"), file_name,
	     elf_header->e_type);
      return 1;
    }

  /* Get information from elf program header.  */
  elf_pheader = (ElfW(Phdr) *) (elf_header->e_phoff + file_contents);
  check_ptr (elf_pheader);

  /* The library is an elf library, now search for soname and
     libc5/libc6.  */
  *flag = FLAG_ELF;

  loadaddr = -1;
  dynamic_addr = 0;
  dynamic_size = 0;
  program_interpreter = NULL;
  for (i = 0, segment = elf_pheader;
       i < elf_header->e_phnum; i++, segment++)
    {
      check_ptr (segment);

      switch (segment->p_type)
	{
	case PT_LOAD:
	  if (loadaddr == (ElfW(Addr)) -1)
	    loadaddr = segment->p_vaddr - segment->p_offset;
	  break;

	case PT_DYNAMIC:
	  if (dynamic_addr)
	    error (0, 0, _("more than one dynamic segment\n"));

	  dynamic_addr = segment->p_offset;
	  dynamic_size = segment->p_filesz;
	  break;

	case PT_INTERP:
	  program_interpreter = (char *) (file_contents + segment->p_offset);
	  check_ptr (program_interpreter);

	  /* Check if this is enough to classify the binary.  */
	  for (j = 0; j < sizeof (interpreters) / sizeof (interpreters [0]);
	       ++j)
	    if (strcmp (program_interpreter, interpreters[j].soname) == 0)
	      {
		*flag = interpreters[j].flag;
		break;
	      }
	  break;

	case PT_NOTE:
	  if (!*osversion && segment->p_filesz >= 32 && segment->p_align >= 4)
	    {
	      ElfW(Word) *abi_note = (ElfW(Word) *) (file_contents
						     + segment->p_offset);
	      ElfW(Addr) size = segment->p_filesz;
	      /* NB: Some PT_NOTE segment may have alignment value of 0
		 or 1.  gABI specifies that PT_NOTE segments should be
		 aligned to 4 bytes in 32-bit objects and to 8 bytes in
		 64-bit objects.  As a Linux extension, we also support
		 4 byte alignment in 64-bit objects.  If p_align is less
		 than 4, we treate alignment as 4 bytes since some note
		 segments have 0 or 1 byte alignment.   */
	      ElfW(Addr) align = segment->p_align;
	      if (align < 4)
		align = 4;
	      else if (align != 4 && align != 8)
		continue;

	      while (abi_note [0] != 4 || abi_note [1] != 16
		     || abi_note [2] != 1
		     || memcmp (abi_note + 3, "GNU", 4) != 0)
		{
		  ElfW(Addr) note_size
		    = ELF_NOTE_NEXT_OFFSET (abi_note[0], abi_note[1],
					    align);

		  if (size - 32 < note_size || note_size == 0)
		    {
		      size = 0;
		      break;
		    }
		  size -= note_size;
		  abi_note = (void *) abi_note + note_size;
		}

	      if (size == 0)
		break;

	      *osversion = (abi_note [4] << 24) |
			   ((abi_note [5] & 0xff) << 16) |
			   ((abi_note [6] & 0xff) << 8) |
			   (abi_note [7] & 0xff);
	    }
	  break;

	default:
	  break;
	}

    }
  if (loadaddr == (ElfW(Addr)) -1)
    {
      /* Very strange. */
      loadaddr = 0;
    }

  /* Now we can read the dynamic sections.  */
  if (dynamic_size == 0)
    return 1;

  dynamic_segment = (ElfW(Dyn) *) (file_contents + dynamic_addr);
  check_ptr (dynamic_segment);

  /* Find the string table.  */
  dynamic_strings = NULL;
  for (dyn_entry = dynamic_segment; dyn_entry->d_tag != DT_NULL;
       ++dyn_entry)
    {
      check_ptr (dyn_entry);
      if (dyn_entry->d_tag == DT_STRTAB)
	{
	  dynamic_strings = (char *) (file_contents + dyn_entry->d_un.d_val - loadaddr);
	  check_ptr (dynamic_strings);
	  break;
	}
    }

  if (dynamic_strings == NULL)
    return 1;

  /* Now read the DT_NEEDED and DT_SONAME entries.  */
  for (dyn_entry = dynamic_segment; dyn_entry->d_tag != DT_NULL;
       ++dyn_entry)
    {
      if (dyn_entry->d_tag == DT_NEEDED || dyn_entry->d_tag == DT_SONAME)
	{
	  char *name = dynamic_strings + dyn_entry->d_un.d_val;
	  check_ptr (name);

	  if (dyn_entry->d_tag == DT_NEEDED)
	    {

	      if (*flag == FLAG_ELF)
		{
		  /* Check if this is enough to classify the binary.  */
		  for (j = 0;
		       j < sizeof (known_libs) / sizeof (known_libs [0]);
		       ++j)
		    if (strcmp (name, known_libs [j].soname) == 0)
		      {
			*flag = known_libs [j].flag;
			break;
		      }
		}
	    }

	  else if (dyn_entry->d_tag == DT_SONAME)
	    *soname = xstrdup (name);

	  /* Do we have everything we need?  */
	  if (*soname && *flag != FLAG_ELF)
	    return 0;
	}
    }

  return 0;
}
