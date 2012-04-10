/* Inline functions for dynamic linking.
   Copyright (C) 1995-2005,2006,2008,2011 Free Software Foundation, Inc.
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

/* This macro is used as a callback from elf_machine_rel{a,} when a
   static TLS reloc is about to be performed.  Since (in dl-load.c) we
   permit dynamic loading of objects that might use such relocs, we
   have to check whether each use is actually doable.  If the object
   whose TLS segment the reference resolves to was allocated space in
   the static TLS block at startup, then it's ok.  Otherwise, we make
   an attempt to allocate it in surplus space on the fly.  If that
   can't be done, we fall back to the error that DF_STATIC_TLS is
   intended to produce.  */
#define CHECK_STATIC_TLS(map, sym_map)					\
    do {								\
      if (__builtin_expect ((sym_map)->l_tls_offset == NO_TLS_OFFSET	\
			    || ((sym_map)->l_tls_offset			\
				== FORCED_DYNAMIC_TLS_OFFSET), 0))	\
	_dl_allocate_static_tls (sym_map);				\
    } while (0)

#define TRY_STATIC_TLS(map, sym_map)					\
    (__builtin_expect ((sym_map)->l_tls_offset				\
		       != FORCED_DYNAMIC_TLS_OFFSET, 1)			\
     && (__builtin_expect ((sym_map)->l_tls_offset != NO_TLS_OFFSET, 1)	\
	 || _dl_try_allocate_static_tls (sym_map) == 0))

int internal_function _dl_try_allocate_static_tls (struct link_map *map);

#include <elf.h>
#include <assert.h>

#ifdef RESOLVE_MAP
/* We pass reloc_addr as a pointer to void, as opposed to a pointer to
   ElfW(Addr), because not all architectures can assume that the
   relocated address is properly aligned, whereas the compiler is
   entitled to assume that a pointer to a type is properly aligned for
   the type.  Even if we cast the pointer back to some other type with
   less strict alignment requirements, the compiler might still
   remember that the pointer was originally more aligned, thereby
   optimizing away alignment tests or using word instructions for
   copying memory, breaking the very code written to handle the
   unaligned cases.  */
# if ! ELF_MACHINE_NO_REL
auto inline void __attribute__((always_inline))
elf_machine_rel (struct link_map *map, const ElfW(Rel) *reloc,
		 const ElfW(Sym) *sym, const struct r_found_version *version,
		 void *const reloc_addr, int skip_ifunc);
auto inline void __attribute__((always_inline))
elf_machine_rel_relative (ElfW(Addr) l_addr, const ElfW(Rel) *reloc,
			  void *const reloc_addr);
# endif
# if ! ELF_MACHINE_NO_RELA
auto inline void __attribute__((always_inline))
elf_machine_rela (struct link_map *map, const ElfW(Rela) *reloc,
		  const ElfW(Sym) *sym, const struct r_found_version *version,
		  void *const reloc_addr, int skip_ifunc);
auto inline void __attribute__((always_inline))
elf_machine_rela_relative (ElfW(Addr) l_addr, const ElfW(Rela) *reloc,
			   void *const reloc_addr);
# endif
# if ELF_MACHINE_NO_RELA || defined ELF_MACHINE_PLT_REL
auto inline void __attribute__((always_inline))
elf_machine_lazy_rel (struct link_map *map,
		      ElfW(Addr) l_addr, const ElfW(Rel) *reloc,
		      int skip_ifunc);
# else
auto inline void __attribute__((always_inline))
elf_machine_lazy_rel (struct link_map *map,
		      ElfW(Addr) l_addr, const ElfW(Rela) *reloc,
		      int skip_ifunc);
# endif
#endif

#include <dl-machine.h>

#ifndef VERSYMIDX
# define VERSYMIDX(sym)	(DT_NUM + DT_THISPROCNUM + DT_VERSIONTAGIDX (sym))
#endif


/* Read the dynamic section at DYN and fill in INFO with indices DT_*.  */
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
	info[dyn->d_tag - DT_LOPROC + DT_NUM] = dyn;
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

      if (l->l_flags_1 & DF_1_NOW)
	info[DT_BIND_NOW] = info[VERSYMIDX (DT_FLAGS_1)];
    }
  if (info[DT_RUNPATH] != NULL)
    /* If both RUNPATH and RPATH are given, the latter is ignored.  */
    info[DT_RPATH] = NULL;
#endif
}

#ifdef RESOLVE_MAP

# ifdef RTLD_BOOTSTRAP
#  define ELF_DURING_STARTUP (1)
# else
#  define ELF_DURING_STARTUP (0)
# endif

/* Get the definitions of `elf_dynamic_do_rel' and `elf_dynamic_do_rela'.
   These functions are almost identical, so we use cpp magic to avoid
   duplicating their code.  It cannot be done in a more general function
   because we must be able to completely inline.  */

/* On some machines, notably SPARC, DT_REL* includes DT_JMPREL in its
   range.  Note that according to the ELF spec, this is completely legal!

   We are guarenteed that we have one of three situations.  Either DT_JMPREL
   comes immediately after DT_REL*, or there is overlap and DT_JMPREL
   consumes precisely the very end of the DT_REL*, or DT_JMPREL and DT_REL*
   are completely separate and there is a gap between them.  */

# define _ELF_DYNAMIC_DO_RELOC(RELOC, reloc, map, do_lazy, skip_ifunc, test_rel) \
  do {									      \
    struct { ElfW(Addr) start, size;					      \
	     __typeof (((ElfW(Dyn) *) 0)->d_un.d_val) nrelative; int lazy; }  \
      ranges[2] = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };			      \
									      \
    if ((map)->l_info[DT_##RELOC])					      \
      {									      \
	ranges[0].start = D_PTR ((map), l_info[DT_##RELOC]);		      \
	ranges[0].size = (map)->l_info[DT_##RELOC##SZ]->d_un.d_val;	      \
	if (map->l_info[VERSYMIDX (DT_##RELOC##COUNT)] != NULL)		      \
	  ranges[0].nrelative						      \
	    = MIN (map->l_info[VERSYMIDX (DT_##RELOC##COUNT)]->d_un.d_val,    \
		   ranges[0].size / sizeof (ElfW(reloc)));		      \
      }									      \
    if ((map)->l_info[DT_PLTREL]					      \
	&& (!test_rel || (map)->l_info[DT_PLTREL]->d_un.d_val == DT_##RELOC)) \
      {									      \
	ElfW(Addr) start = D_PTR ((map), l_info[DT_JMPREL]);		      \
	ElfW(Addr) size = (map)->l_info[DT_PLTRELSZ]->d_un.d_val;	      \
									      \
	if (ranges[0].start + ranges[0].size == (start + size))		      \
	  ranges[0].size -= size;					      \
	if (! ELF_DURING_STARTUP && ((do_lazy) || ranges[0].size == 0))	      \
	  {								      \
	    ranges[1].start = start;					      \
	    ranges[1].size = size;					      \
	    ranges[1].lazy = (do_lazy);					      \
	  }								      \
	else								      \
	  {								      \
	    /* Combine processing the sections.  */			      \
	    ranges[0].size += size;					      \
	  }								      \
      }									      \
									      \
    if (ELF_DURING_STARTUP)						      \
      elf_dynamic_do_##reloc ((map), ranges[0].start, ranges[0].size,	      \
			      ranges[0].nrelative, 0, skip_ifunc);	      \
    else								      \
      {									      \
	int ranges_index;						      \
	for (ranges_index = 0; ranges_index < 2; ++ranges_index)	      \
	  elf_dynamic_do_##reloc ((map),				      \
				  ranges[ranges_index].start,		      \
				  ranges[ranges_index].size,		      \
				  ranges[ranges_index].nrelative,	      \
				  ranges[ranges_index].lazy,		      \
				  skip_ifunc);				      \
      }									      \
  } while (0)

# if ELF_MACHINE_NO_REL || ELF_MACHINE_NO_RELA
#  define _ELF_CHECK_REL 0
# else
#  define _ELF_CHECK_REL 1
# endif

# if ! ELF_MACHINE_NO_REL
#  include "do-rel.h"
#  define ELF_DYNAMIC_DO_REL(map, lazy, skip_ifunc) \
  _ELF_DYNAMIC_DO_RELOC (REL, Rel, map, lazy, skip_ifunc, _ELF_CHECK_REL)
# else
#  define ELF_DYNAMIC_DO_REL(map, lazy, skip_ifunc) /* Nothing to do.  */
# endif

# if ! ELF_MACHINE_NO_RELA
#  define DO_RELA
#  include "do-rel.h"
#  define ELF_DYNAMIC_DO_RELA(map, lazy, skip_ifunc) \
  _ELF_DYNAMIC_DO_RELOC (RELA, Rela, map, lazy, skip_ifunc, _ELF_CHECK_REL)
# else
#  define ELF_DYNAMIC_DO_RELA(map, lazy, skip_ifunc) /* Nothing to do.  */
# endif

/* This can't just be an inline function because GCC is too dumb
   to inline functions containing inlines themselves.  */
# define ELF_DYNAMIC_RELOCATE(map, lazy, consider_profile, skip_ifunc) \
  do {									      \
    int edr_lazy = elf_machine_runtime_setup ((map), (lazy),		      \
					      (consider_profile));	      \
    ELF_DYNAMIC_DO_REL ((map), edr_lazy, skip_ifunc);			      \
    ELF_DYNAMIC_DO_RELA ((map), edr_lazy, skip_ifunc);			      \
  } while (0)

#endif
