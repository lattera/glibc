#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <bits/wordsize.h>
#include <gnu/lib-names.h>


unsigned int
la_version (unsigned int v)
{
  setlinebuf (stdout);

  printf ("version: %u\n", v);

  char buf[20];
  sprintf (buf, "%u", v);

  return v;
}

void
la_activity (uintptr_t *cookie, unsigned int flag)
{
  if (flag == LA_ACT_CONSISTENT)
    printf ("activity: consistent\n");
  else if (flag == LA_ACT_ADD)
    printf ("activity: add\n");
  else if (flag == LA_ACT_DELETE)
    printf ("activity: delete\n");
  else
    printf ("activity: unknown activity %u\n", flag);
}

char *
la_objsearch (const char *name, uintptr_t *cookie, unsigned int flag)
{
  char buf[100];
  const char *flagstr;
  if (flag == LA_SER_ORIG)
    flagstr = "LA_SET_ORIG";
  else if (flag == LA_SER_LIBPATH)
    flagstr = "LA_SER_LIBPATH";
  else if (flag == LA_SER_RUNPATH)
    flagstr = "LA_SER_RUNPATH";
  else if (flag == LA_SER_CONFIG)
    flagstr = "LA_SER_CONFIG";
  else if (flag == LA_SER_DEFAULT)
    flagstr = "LA_SER_DEFAULT";
  else if (flag == LA_SER_SECURE)
    flagstr = "LA_SER_SECURE";
  else
    {
       sprintf (buf, "unknown flag %d", flag);
       flagstr = buf;
    }
  printf ("objsearch: %s, %s\n", name, flagstr);

  return (char *) name;
}

unsigned int
la_objopen (struct link_map *l, Lmid_t lmid, uintptr_t *cookie)
{
  printf ("objopen: %ld, %s\n", lmid, l->l_name);

  return 3;
}

void
la_preinit (uintptr_t *cookie)
{
  printf ("preinit\n");
}

unsigned int
la_objclose  (uintptr_t *cookie)
{
  printf ("objclose\n");
  return 0;
}

uintptr_t
la_symbind32 (Elf32_Sym *sym, unsigned int ndx, uintptr_t *refcook,
	      uintptr_t *defcook, unsigned int *flags, const char *symname)
{
  printf ("symbind32: symname=%s, st_value=%#lx, ndx=%u, flags=%u\n",
	  symname, (long int) sym->st_value, ndx, *flags);

  return sym->st_value;
}

uintptr_t
la_symbind64 (Elf64_Sym *sym, unsigned int ndx, uintptr_t *refcook,
	      uintptr_t *defcook, unsigned int *flags, const char *symname)
{
  printf ("symbind64: symname=%s, st_value=%#lx, ndx=%u, flags=%u\n",
	  symname, (long int) sym->st_value, ndx, *flags);

  return sym->st_value;
}

#ifdef __i386__
Elf32_Addr
la_i86_gnu_pltenter (Elf32_Sym *sym, unsigned int ndx, uintptr_t *refcook,
		     uintptr_t *defcook, La_i86_regs *regs,
		     unsigned int *flags, const char *symname,
		     long int *framesizep)
{
  printf ("i86_pltenter: symname=%s, st_value=%#lx, ndx=%u, flags=%u\n",
	  symname, (long int) sym->st_value, ndx, *flags);

  return sym->st_value;
}

unsigned int
la_i86_gnu_pltexit (Elf32_Sym *sym, unsigned int ndx, uintptr_t *refcook,
		    uintptr_t *defcook, const La_i86_regs *inregs,
		    La_i86_retval *outregs, const char *symname)
{
  printf ("i86_pltexit: symname=%s, st_value=%#lx, ndx=%u, retval=%tu\n",
	  symname, (long int) sym->st_value, ndx, outregs->lrv_eax);

  return 0;
}
#elif defined __x86_64__
Elf64_Addr
la_x86_64_gnu_pltenter (Elf64_Sym *sym, unsigned int ndx, uintptr_t *refcook,
			uintptr_t *defcook, La_x86_64_regs *regs,
			unsigned int *flags, const char *symname,
			long int *framesizep)
{
  printf ("x86_64_pltenter: symname=%s, st_value=%#lx, ndx=%u, flags=%u\n",
	  symname, (long int) sym->st_value, ndx, *flags);

  return sym->st_value;
}

unsigned int
la_x86_64_gnu_pltexit (Elf64_Sym *sym, unsigned int ndx, uintptr_t *refcook,
		       uintptr_t *defcook, const La_x86_64_regs *inregs,
		       La_x86_64_retval *outregs, const char *symname)
{
  printf ("x86_64_pltexit: symname=%s, st_value=%#lx, ndx=%u, retval=%tu\n",
	  symname, (long int) sym->st_value, ndx, outregs->lrv_rax);

  return 0;
}
#elif defined __powerpc__ && __WORDSIZE == 32
Elf32_Addr
la_ppc32_gnu_pltenter (Elf32_Sym *sym, unsigned int ndx, uintptr_t *refcook,
		       uintptr_t *defcook, La_ppc32_regs *regs,
		       unsigned int *flags, const char *symname,
		       long int *framesizep)
{
  printf ("ppc32_pltenter: symname=%s, st_value=%#lx, ndx=%u, flags=%u\n",
	  symname, (long int) sym->st_value, ndx, *flags);

  return sym->st_value;
}

unsigned int
la_ppc32_gnu_pltexit (Elf32_Sym *sym, unsigned int ndx, uintptr_t *refcook,
		      uintptr_t *defcook, const La_ppc32_regs *inregs,
		      La_ppc32_retval *outregs, const char *symname)
{
  printf ("ppc32_pltexit: symname=%s, st_value=%#lx, ndx=%u, retval=%tu\n",
	  symname, (long int) sym->st_value, ndx, outregs->lrv_r3);

  return 0;
}
#elif defined __powerpc__ && __WORDSIZE == 64
Elf64_Addr
la_ppc64_gnu_pltenter (Elf64_Sym *sym, unsigned int ndx, uintptr_t *refcook,
		       uintptr_t *defcook, La_ppc64_regs *regs,
		       unsigned int *flags, const char *symname,
		       long int *framesizep)
{
  printf ("ppc64_pltenter: symname=%s, st_value=%#lx, ndx=%u, flags=%u\n",
	  symname, (long int) sym->st_value, ndx, *flags);

  return sym->st_value;
}

unsigned int
la_ppc64_gnu_pltexit (Elf64_Sym *sym, unsigned int ndx, uintptr_t *refcook,
		      uintptr_t *defcook, const La_ppc64_regs *inregs,
		      La_ppc64_retval *outregs, const char *symname)
{
  printf ("ppc64_pltexit: symname=%s, st_value=%#lx, ndx=%u, retval=%tu\n",
	  symname, (long int) sym->st_value, ndx, outregs->lrv_r3);

  return 0;
}
#elif defined __sh__
uintptr_t
la_sh_gnu_pltenter (Elf32_Sym *sym, unsigned int ndx, uintptr_t *refcook,
		    uintptr_t *defcook, La_sh_regs *regs,
		    unsigned int *flags, const char *symname,
		    long int *framesizep)
{
  printf ("sh_pltenter: symname=%s, st_value=%#lx, ndx=%u, flags=%u\n",
	  symname, (long int) sym->st_value, ndx, *flags);

  return sym->st_value;
}

unsigned int
la_sh_gnu_pltexit (Elf32_Sym *sym, unsigned int ndx, uintptr_t *refcook,
		   uintptr_t *defcook, const La_sh_regs *inregs,
		   La_sh_retval *outregs, const char *symname)
{
  printf ("sh_pltexit: symname=%s, st_value=%#lx, ndx=%u, retval=%tu\n",
	  symname, (long int) sym->st_value, ndx, outregs->lrv_r0);

  return 0;
}
#elif defined __mc68000__
Elf32_Addr
la_m68k_gnu_pltenter (Elf32_Sym *sym, unsigned int ndx, uintptr_t *refcook,
		      uintptr_t *defcook, La_m68k_regs *regs,
		      unsigned int *flags, const char *symname,
		      long int *framesizep)
{
  printf ("m68k_pltenter: symname=%s, st_value=%#lx, ndx=%u, flags=%u\n",
	  symname, (long int) sym->st_value, ndx, *flags);

  return sym->st_value;
}

unsigned int
la_m68k_gnu_pltexit (Elf32_Sym *sym, unsigned int ndx, uintptr_t *refcook,
		     uintptr_t *defcook, const La_m68k_regs *inregs,
		     La_m68k_retval *outregs, const char *symname)
{
  printf ("m68k_pltexit: symname=%s, st_value=%#lx, ndx=%u, retval=%tu\n",
	  symname, (long int) sym->st_value, ndx, outregs->lrv_d0);

  return 0;
}
#else
# error "architecture specific code needed"
#endif
