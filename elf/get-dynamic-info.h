/* Read the dynamic section at DYN and fill in INFO with indices DT_*.
   Copyright (C) 2012-2016 Free Software Foundation, Inc.
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

#include <assert.h>
#include <libc-internal.h>

#ifndef RESOLVE_MAP
static
#else
auto
#endif
inline void __attribute__ ((unused, always_inline))
elf_get_dynamic_info (struct link_map *l, ElfW(Dyn) *temp)
{
  ElfW(Dyn) *dyn = l->l_ld;
  ElfW(Dyn) **info;
#if __ELF_NATIVE_CLASS == 32
  typedef Elf32_Word d_tag_utype;
#elif __ELF_NATIVE_CLASS == 64
  typedef Elf64_Xword d_tag_utype;
#endif

#ifndef RTLD_BOOTSTRAP
  if (dyn == NULL)
    return;
#endif

  info = l->l_info;

  while (dyn->d_tag != DT_NULL)
    {
      if ((d_tag_utype) dyn->d_tag < DT_NUM)
	info[dyn->d_tag] = dyn;
      else if (dyn->d_tag >= DT_LOPROC &&
	       dyn->d_tag < DT_LOPROC + DT_THISPROCNUM)
	{
	  /* This does not violate the array bounds of l->l_info, but
	     gcc 4.6 on sparc somehow does not see this.  */
	  DIAG_PUSH_NEEDS_COMMENT;
	  DIAG_IGNORE_NEEDS_COMMENT (4.6,
				     "-Warray-bounds");
	  info[dyn->d_tag - DT_LOPROC + DT_NUM] = dyn;
	  DIAG_POP_NEEDS_COMMENT;
	}
      else if ((d_tag_utype) DT_VERSIONTAGIDX (dyn->d_tag) < DT_VERSIONTAGNUM)
	info[VERSYMIDX (dyn->d_tag)] = dyn;
      else if ((d_tag_utype) DT_EXTRATAGIDX (dyn->d_tag) < DT_EXTRANUM)
	info[DT_EXTRATAGIDX (dyn->d_tag) + DT_NUM + DT_THISPROCNUM
	     + DT_VERSIONTAGNUM] = dyn;
      else if ((d_tag_utype) DT_VALTAGIDX (dyn->d_tag) < DT_VALNUM)
	info[DT_VALTAGIDX (dyn->d_tag) + DT_NUM + DT_THISPROCNUM
	     + DT_VERSIONTAGNUM + DT_EXTRANUM] = dyn;
      else if ((d_tag_utype) DT_ADDRTAGIDX (dyn->d_tag) < DT_ADDRNUM)
	info[DT_ADDRTAGIDX (dyn->d_tag) + DT_NUM + DT_THISPROCNUM
	     + DT_VERSIONTAGNUM + DT_EXTRANUM + DT_VALNUM] = dyn;
      ++dyn;
    }

#define DL_RO_DYN_TEMP_CNT	8

#ifndef DL_RO_DYN_SECTION
  /* Don't adjust .dynamic unnecessarily.  */
  if (l->l_addr != 0)
    {
      ElfW(Addr) l_addr = l->l_addr;
      int cnt = 0;

# define ADJUST_DYN_INFO(tag) \
      do								      \
	if (info[tag] != NULL)						      \
	  {								      \
	    if (temp)							      \
	      {								      \
		temp[cnt].d_tag = info[tag]->d_tag;			      \
		temp[cnt].d_un.d_ptr = info[tag]->d_un.d_ptr + l_addr;	      \
		info[tag] = temp + cnt++;				      \
	      }								      \
	    else							      \
	      info[tag]->d_un.d_ptr += l_addr;				      \
	  }								      \
      while (0)

      ADJUST_DYN_INFO (DT_HASH);
      ADJUST_DYN_INFO (DT_PLTGOT);
      ADJUST_DYN_INFO (DT_STRTAB);
      ADJUST_DYN_INFO (DT_SYMTAB);
# if ! ELF_MACHINE_NO_RELA
      ADJUST_DYN_INFO (DT_RELA);
# endif
# if ! ELF_MACHINE_NO_REL
      ADJUST_DYN_INFO (DT_REL);
# endif
      ADJUST_DYN_INFO (DT_JMPREL);
      ADJUST_DYN_INFO (VERSYMIDX (DT_VERSYM));
      ADJUST_DYN_INFO (DT_ADDRTAGIDX (DT_GNU_HASH) + DT_NUM + DT_THISPROCNUM
		       + DT_VERSIONTAGNUM + DT_EXTRANUM + DT_VALNUM);
# undef ADJUST_DYN_INFO
      assert (cnt <= DL_RO_DYN_TEMP_CNT);
    }
#endif
  if (info[DT_PLTREL] != NULL)
    {
#if ELF_MACHINE_NO_RELA
      assert (info[DT_PLTREL]->d_un.d_val == DT_REL);
#elif ELF_MACHINE_NO_REL
      assert (info[DT_PLTREL]->d_un.d_val == DT_RELA);
#else
      assert (info[DT_PLTREL]->d_un.d_val == DT_REL
	      || info[DT_PLTREL]->d_un.d_val == DT_RELA);
#endif
    }
#if ! ELF_MACHINE_NO_RELA
  if (info[DT_RELA] != NULL)
    assert (info[DT_RELAENT]->d_un.d_val == sizeof (ElfW(Rela)));
# endif
# if ! ELF_MACHINE_NO_REL
  if (info[DT_REL] != NULL)
    assert (info[DT_RELENT]->d_un.d_val == sizeof (ElfW(Rel)));
#endif
#ifdef RTLD_BOOTSTRAP
  /* Only the bind now flags are allowed.  */
  assert (info[VERSYMIDX (DT_FLAGS_1)] == NULL
	  || (info[VERSYMIDX (DT_FLAGS_1)]->d_un.d_val & ~DF_1_NOW) == 0);
  assert (info[DT_FLAGS] == NULL
	  || (info[DT_FLAGS]->d_un.d_val & ~DF_BIND_NOW) == 0);
  /* Flags must not be set for ld.so.  */
  assert (info[DT_RUNPATH] == NULL);
  assert (info[DT_RPATH] == NULL);
#else
  if (info[DT_FLAGS] != NULL)
    {
      /* Flags are used.  Translate to the old form where available.
	 Since these l_info entries are only tested for NULL pointers it
	 is ok if they point to the DT_FLAGS entry.  */
      l->l_flags = info[DT_FLAGS]->d_un.d_val;

      if (l->l_flags & DF_SYMBOLIC)
	info[DT_SYMBOLIC] = info[DT_FLAGS];
      if (l->l_flags & DF_TEXTREL)
	info[DT_TEXTREL] = info[DT_FLAGS];
      if (l->l_flags & DF_BIND_NOW)
	info[DT_BIND_NOW] = info[DT_FLAGS];
    }
  if (info[VERSYMIDX (DT_FLAGS_1)] != NULL)
    {
      l->l_flags_1 = info[VERSYMIDX (DT_FLAGS_1)]->d_un.d_val;

      /* Only DT_1_SUPPORTED_MASK bits are supported, and we would like
	 to assert this, but we can't. Users have been setting
	 unsupported DF_1_* flags for a long time and glibc has ignored
	 them. Therefore to avoid breaking existing applications the
	 best we can do is add a warning during debugging with the
	 intent of notifying the user of the problem.  */
      if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_FILES, 0)
	  && l->l_flags_1 & ~DT_1_SUPPORTED_MASK)
	_dl_debug_printf ("\nWARNING: Unsupported flag value(s) of 0x%x in DT_FLAGS_1.\n",
			  l->l_flags_1 & ~DT_1_SUPPORTED_MASK);

      if (l->l_flags_1 & DF_1_NOW)
	info[DT_BIND_NOW] = info[VERSYMIDX (DT_FLAGS_1)];
    }
  if (info[DT_RUNPATH] != NULL)
    /* If both RUNPATH and RPATH are given, the latter is ignored.  */
    info[DT_RPATH] = NULL;
#endif
}
