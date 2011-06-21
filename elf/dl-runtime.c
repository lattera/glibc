/* On-demand PLT fixup for shared objects.
   Copyright (C) 1995-2009, 2010, 2011 Free Software Foundation, Inc.
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

#define IN_DL_RUNTIME 1		/* This can be tested in dl-machine.h.  */

#include <alloca.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <ldsodefs.h>
#include <sysdep-cancel.h>
#include "dynamic-link.h"
#include <tls.h>
#include <dl-irel.h>


#if (!defined ELF_MACHINE_NO_RELA && !defined ELF_MACHINE_PLT_REL) \
    || ELF_MACHINE_NO_REL
# define PLTREL  ElfW(Rela)
#else
# define PLTREL  ElfW(Rel)
#endif

#ifndef VERSYMIDX
# define VERSYMIDX(sym)	(DT_NUM + DT_THISPROCNUM + DT_VERSIONTAGIDX (sym))
#endif

/* The fixup functions might have need special attributes.  If none
   are provided define the macro as empty.  */
#ifndef ARCH_FIXUP_ATTRIBUTE
# define ARCH_FIXUP_ATTRIBUTE
#endif

#ifndef reloc_offset
# define reloc_offset reloc_arg
# define reloc_index  reloc_arg / sizeof (PLTREL)
#endif



/* This function is called through a special trampoline from the PLT the
   first time each PLT entry is called.  We must perform the relocation
   specified in the PLT of the given shared object, and return the resolved
   function address to the trampoline, which will restart the original call
   to that address.  Future calls will bounce directly from the PLT to the
   function.  */

#ifndef ELF_MACHINE_NO_PLT
DL_FIXUP_VALUE_TYPE
__attribute ((noinline)) ARCH_FIXUP_ATTRIBUTE
_dl_fixup (
# ifdef ELF_MACHINE_RUNTIME_FIXUP_ARGS
	   ELF_MACHINE_RUNTIME_FIXUP_ARGS,
# endif
	   /* GKM FIXME: Fix trampoline to pass bounds so we can do
	      without the `__unbounded' qualifier.  */
	   struct link_map *__unbounded l, ElfW(Word) reloc_arg)
{
  const ElfW(Sym) *const symtab
    = (const void *) D_PTR (l, l_info[DT_SYMTAB]);
  const char *strtab = (const void *) D_PTR (l, l_info[DT_STRTAB]);

  const PLTREL *const reloc
    = (const void *) (D_PTR (l, l_info[DT_JMPREL]) + reloc_offset);
  const ElfW(Sym) *sym = &symtab[ELFW(R_SYM) (reloc->r_info)];
  void *const rel_addr = (void *)(l->l_addr + reloc->r_offset);
  lookup_t result;
  DL_FIXUP_VALUE_TYPE value;

  /* Sanity check that we're really looking at a PLT relocation.  */
  assert (ELFW(R_TYPE)(reloc->r_info) == ELF_MACHINE_JMP_SLOT);

   /* Look up the target symbol.  If the normal lookup rules are not
      used don't look in the global scope.  */
  if (__builtin_expect (ELFW(ST_VISIBILITY) (sym->st_other), 0) == 0)
    {
      const struct r_found_version *version = NULL;

      if (l->l_info[VERSYMIDX (DT_VERSYM)] != NULL)
	{
	  const ElfW(Half) *vernum =
	    (const void *) D_PTR (l, l_info[VERSYMIDX (DT_VERSYM)]);
	  ElfW(Half) ndx = vernum[ELFW(R_SYM) (reloc->r_info)] & 0x7fff;
	  version = &l->l_versions[ndx];
	  if (version->hash == 0)
	    version = NULL;
	}

      /* We need to keep the scope around so do some locking.  This is
	 not necessary for objects which cannot be unloaded or when
	 we are not using any threads (yet).  */
      int flags = DL_LOOKUP_ADD_DEPENDENCY;
      if (!RTLD_SINGLE_THREAD_P)
	{
	  THREAD_GSCOPE_SET_FLAG ();
	  flags |= DL_LOOKUP_GSCOPE_LOCK;
	}

#ifdef RTLD_ENABLE_FOREIGN_CALL
      RTLD_ENABLE_FOREIGN_CALL;
#endif

      result = _dl_lookup_symbol_x (strtab + sym->st_name, l, &sym, l->l_scope,
				    version, ELF_RTYPE_CLASS_PLT, flags, NULL);

      /* We are done with the global scope.  */
      if (!RTLD_SINGLE_THREAD_P)
	THREAD_GSCOPE_RESET_FLAG ();

#ifdef RTLD_FINALIZE_FOREIGN_CALL
      RTLD_FINALIZE_FOREIGN_CALL;
#endif

      /* Currently result contains the base load address (or link map)
	 of the object that defines sym.  Now add in the symbol
	 offset.  */
      value = DL_FIXUP_MAKE_VALUE (result,
				   sym ? (LOOKUP_VALUE_ADDRESS (result)
					  + sym->st_value) : 0);
    }
  else
    {
      /* We already found the symbol.  The module (and therefore its load
	 address) is also known.  */
      value = DL_FIXUP_MAKE_VALUE (l, l->l_addr + sym->st_value);
      result = l;
    }

  /* And now perhaps the relocation addend.  */
  value = elf_machine_plt_value (l, reloc, value);

  if (sym != NULL
      && __builtin_expect (ELFW(ST_TYPE) (sym->st_info) == STT_GNU_IFUNC, 0))
    value = elf_ifunc_invoke (DL_FIXUP_VALUE_ADDR (value));

  /* Finally, fix up the plt itself.  */
  if (__builtin_expect (GLRO(dl_bind_not), 0))
    return value;

  return elf_machine_fixup_plt (l, result, reloc, rel_addr, value);
}
#endif

#if !defined PROF && !defined ELF_MACHINE_NO_PLT && !__BOUNDED_POINTERS__
DL_FIXUP_VALUE_TYPE
__attribute ((noinline)) ARCH_FIXUP_ATTRIBUTE
_dl_profile_fixup (
#ifdef ELF_MACHINE_RUNTIME_FIXUP_ARGS
		   ELF_MACHINE_RUNTIME_FIXUP_ARGS,
#endif
		   struct link_map *l, ElfW(Word) reloc_arg,
		   ElfW(Addr) retaddr, void *regs, long int *framesizep)
{
  void (*mcount_fct) (ElfW(Addr), ElfW(Addr)) = INTUSE(_dl_mcount);

  /* This is the address in the array where we store the result of previous
     relocations.  */
  struct reloc_result *reloc_result = &l->l_reloc_result[reloc_index];
  DL_FIXUP_VALUE_TYPE *resultp = &reloc_result->addr;

  DL_FIXUP_VALUE_TYPE value = *resultp;
  if (DL_FIXUP_VALUE_CODE_ADDR (value) == 0)
    {
      /* This is the first time we have to relocate this object.  */
      const ElfW(Sym) *const symtab
	= (const void *) D_PTR (l, l_info[DT_SYMTAB]);
      const char *strtab = (const char *) D_PTR (l, l_info[DT_STRTAB]);

      const PLTREL *const reloc
	= (const void *) (D_PTR (l, l_info[DT_JMPREL]) + reloc_offset);
      const ElfW(Sym) *refsym = &symtab[ELFW(R_SYM) (reloc->r_info)];
      const ElfW(Sym) *defsym = refsym;
      lookup_t result;

      /* Sanity check that we're really looking at a PLT relocation.  */
      assert (ELFW(R_TYPE)(reloc->r_info) == ELF_MACHINE_JMP_SLOT);

      /* Look up the target symbol.  If the symbol is marked STV_PROTECTED
	 don't look in the global scope.  */
      if (__builtin_expect (ELFW(ST_VISIBILITY) (refsym->st_other), 0) == 0)
	{
	  const struct r_found_version *version = NULL;

	  if (l->l_info[VERSYMIDX (DT_VERSYM)] != NULL)
	    {
	      const ElfW(Half) *vernum =
		(const void *) D_PTR (l, l_info[VERSYMIDX (DT_VERSYM)]);
	      ElfW(Half) ndx = vernum[ELFW(R_SYM) (reloc->r_info)] & 0x7fff;
	      version = &l->l_versions[ndx];
	      if (version->hash == 0)
		version = NULL;
	    }

	  /* We need to keep the scope around so do some locking.  This is
	     not necessary for objects which cannot be unloaded or when
	     we are not using any threads (yet).  */
	  int flags = DL_LOOKUP_ADD_DEPENDENCY;
	  if (!RTLD_SINGLE_THREAD_P)
	    {
	      THREAD_GSCOPE_SET_FLAG ();
	      flags |= DL_LOOKUP_GSCOPE_LOCK;
	    }

	  result = _dl_lookup_symbol_x (strtab + refsym->st_name, l,
					&defsym, l->l_scope, version,
					ELF_RTYPE_CLASS_PLT, flags, NULL);

	  /* We are done with the global scope.  */
	  if (!RTLD_SINGLE_THREAD_P)
	    THREAD_GSCOPE_RESET_FLAG ();

	  /* Currently result contains the base load address (or link map)
	     of the object that defines sym.  Now add in the symbol
	     offset.  */
	  value = DL_FIXUP_MAKE_VALUE (result,
				       defsym != NULL
				       ? LOOKUP_VALUE_ADDRESS (result)
					 + defsym->st_value : 0);

	  if (defsym != NULL
	      && __builtin_expect (ELFW(ST_TYPE) (defsym->st_info)
				   == STT_GNU_IFUNC, 0))
	    value = elf_ifunc_invoke (DL_FIXUP_VALUE_ADDR (value));
	}
      else
	{
	  /* We already found the symbol.  The module (and therefore its load
	     address) is also known.  */
	  value = DL_FIXUP_MAKE_VALUE (l, l->l_addr + refsym->st_value);

	  if (__builtin_expect (ELFW(ST_TYPE) (refsym->st_info)
				== STT_GNU_IFUNC, 0))
	    value = elf_ifunc_invoke (DL_FIXUP_VALUE_ADDR (value));

	  result = l;
	}
      /* And now perhaps the relocation addend.  */
      value = elf_machine_plt_value (l, reloc, value);

#ifdef SHARED
      /* Auditing checkpoint: we have a new binding.  Provide the
	 auditing libraries the possibility to change the value and
	 tell us whether further auditing is wanted.  */
      if (defsym != NULL && GLRO(dl_naudit) > 0)
	{
	  reloc_result->bound = result;
	  /* Compute index of the symbol entry in the symbol table of
	     the DSO with the definition.  */
	  reloc_result->boundndx = (defsym
				    - (ElfW(Sym) *) D_PTR (result,
							   l_info[DT_SYMTAB]));

	  /* Determine whether any of the two participating DSOs is
	     interested in auditing.  */
	  if ((l->l_audit_any_plt | result->l_audit_any_plt) != 0)
	    {
	      unsigned int flags = 0;
	      struct audit_ifaces *afct = GLRO(dl_audit);
	      /* Synthesize a symbol record where the st_value field is
		 the result.  */
	      ElfW(Sym) sym = *defsym;
	      sym.st_value = DL_FIXUP_VALUE_ADDR (value);

	      /* Keep track whether there is any interest in tracing
		 the call in the lower two bits.  */
	      assert (DL_NNS * 2 <= sizeof (reloc_result->flags) * 8);
	      assert ((LA_SYMB_NOPLTENTER | LA_SYMB_NOPLTEXIT) == 3);
	      reloc_result->enterexit = LA_SYMB_NOPLTENTER | LA_SYMB_NOPLTEXIT;

	      const char *strtab2 = (const void *) D_PTR (result,
							  l_info[DT_STRTAB]);

	      for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
		{
		  /* XXX Check whether both DSOs must request action or
		     only one */
		  if ((l->l_audit[cnt].bindflags & LA_FLG_BINDFROM) != 0
		      && (result->l_audit[cnt].bindflags & LA_FLG_BINDTO) != 0)
		    {
		      if (afct->symbind != NULL)
			{
			  uintptr_t new_value
			    = afct->symbind (&sym, reloc_result->boundndx,
					     &l->l_audit[cnt].cookie,
					     &result->l_audit[cnt].cookie,
					     &flags,
					     strtab2 + defsym->st_name);
			  if (new_value != (uintptr_t) sym.st_value)
			    {
			      flags |= LA_SYMB_ALTVALUE;
			      sym.st_value = new_value;
			    }
			}

		      /* Remember the results for every audit library and
			 store a summary in the first two bits.  */
		      reloc_result->enterexit
			&= flags & (LA_SYMB_NOPLTENTER | LA_SYMB_NOPLTEXIT);
		      reloc_result->enterexit
			|= ((flags & (LA_SYMB_NOPLTENTER | LA_SYMB_NOPLTEXIT))
			    << ((cnt + 1) * 2));
		    }
		  else
		    /* If the bind flags say this auditor is not interested,
		       set the bits manually.  */
		    reloc_result->enterexit
		      |= ((LA_SYMB_NOPLTENTER | LA_SYMB_NOPLTEXIT)
			  << ((cnt + 1) * 2));

		  afct = afct->next;
		}

	      reloc_result->flags = flags;
	      value = DL_FIXUP_ADDR_VALUE (sym.st_value);
	    }
	  else
	    /* Set all bits since this symbol binding is not interesting.  */
	    reloc_result->enterexit = (1u << DL_NNS) - 1;
	}
#endif

      /* Store the result for later runs.  */
      if (__builtin_expect (! GLRO(dl_bind_not), 1))
	*resultp = value;
    }

  /* By default we do not call the pltexit function.  */
  long int framesize = -1;

#ifdef SHARED
  /* Auditing checkpoint: report the PLT entering and allow the
     auditors to change the value.  */
  if (DL_FIXUP_VALUE_CODE_ADDR (value) != 0 && GLRO(dl_naudit) > 0
      /* Don't do anything if no auditor wants to intercept this call.  */
      && (reloc_result->enterexit & LA_SYMB_NOPLTENTER) == 0)
    {
      ElfW(Sym) *defsym = ((ElfW(Sym) *) D_PTR (reloc_result->bound,
						l_info[DT_SYMTAB])
			   + reloc_result->boundndx);

      /* Set up the sym parameter.  */
      ElfW(Sym) sym = *defsym;
      sym.st_value = DL_FIXUP_VALUE_ADDR (value);

      /* Get the symbol name.  */
      const char *strtab = (const void *) D_PTR (reloc_result->bound,
						 l_info[DT_STRTAB]);
      const char *symname = strtab + sym.st_name;

      /* Keep track of overwritten addresses.  */
      unsigned int flags = reloc_result->flags;

      struct audit_ifaces *afct = GLRO(dl_audit);
      for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
	{
	  if (afct->ARCH_LA_PLTENTER != NULL
	      && (reloc_result->enterexit
		  & (LA_SYMB_NOPLTENTER << (2 * (cnt + 1)))) == 0)
	    {
	      long int new_framesize = -1;
	      uintptr_t new_value
		= afct->ARCH_LA_PLTENTER (&sym, reloc_result->boundndx,
					  &l->l_audit[cnt].cookie,
					  &reloc_result->bound->l_audit[cnt].cookie,
					  regs, &flags, symname,
					  &new_framesize);
	      if (new_value != (uintptr_t) sym.st_value)
		{
		  flags |= LA_SYMB_ALTVALUE;
		  sym.st_value = new_value;
		}

	      /* Remember the results for every audit library and
		 store a summary in the first two bits.  */
	      reloc_result->enterexit
		|= ((flags & (LA_SYMB_NOPLTENTER | LA_SYMB_NOPLTEXIT))
		    << (2 * (cnt + 1)));

	      if ((reloc_result->enterexit & (LA_SYMB_NOPLTEXIT
					      << (2 * (cnt + 1))))
		  == 0 && new_framesize != -1 && framesize != -2)
		{
		  /* If this is the first call providing information,
		     use it.  */
		  if (framesize == -1)
		    framesize = new_framesize;
		  /* If two pltenter calls provide conflicting information,
		     use the larger value.  */
		  else if (new_framesize != framesize)
		    framesize = MAX (new_framesize, framesize);
		}
	    }

	  afct = afct->next;
	}

      value = DL_FIXUP_ADDR_VALUE (sym.st_value);
    }
#endif

  /* Store the frame size information.  */
  *framesizep = framesize;

  (*mcount_fct) (retaddr, DL_FIXUP_VALUE_CODE_ADDR (value));

  return value;
}

#endif /* PROF && ELF_MACHINE_NO_PLT */


#include <stdio.h>
void
ARCH_FIXUP_ATTRIBUTE
_dl_call_pltexit (struct link_map *l, ElfW(Word) reloc_arg,
		  const void *inregs, void *outregs)
{
#ifdef SHARED
  /* This is the address in the array where we store the result of previous
     relocations.  */
  // XXX Maybe the bound information must be stored on the stack since
  // XXX with bind_not a new value could have been stored in the meantime.
  struct reloc_result *reloc_result = &l->l_reloc_result[reloc_index];
  ElfW(Sym) *defsym = ((ElfW(Sym) *) D_PTR (reloc_result->bound,
					    l_info[DT_SYMTAB])
		       + reloc_result->boundndx);

  /* Set up the sym parameter.  */
  ElfW(Sym) sym = *defsym;
  sym.st_value = DL_FIXUP_VALUE_ADDR (reloc_result->addr);

  /* Get the symbol name.  */
  const char *strtab = (const void *) D_PTR (reloc_result->bound,
					     l_info[DT_STRTAB]);
  const char *symname = strtab + sym.st_name;

  struct audit_ifaces *afct = GLRO(dl_audit);
  for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
    {
      if (afct->ARCH_LA_PLTEXIT != NULL
	  && (reloc_result->enterexit
	      & (LA_SYMB_NOPLTEXIT >> (2 * cnt))) == 0)
	{
	  afct->ARCH_LA_PLTEXIT (&sym, reloc_result->boundndx,
				 &l->l_audit[cnt].cookie,
				 &reloc_result->bound->l_audit[cnt].cookie,
				 inregs, outregs, symname);
	}

      afct = afct->next;
    }
#endif
}
