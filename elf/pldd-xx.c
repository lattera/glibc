/* Copyright (C) 2011-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gmail.com>, 2011.

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

#define E(name) E_(name, CLASS)
#define E_(name, cl) E__(name, cl)
#define E__(name, cl) name##cl
#define EW(type) EW_(Elf, CLASS, type)
#define EW_(e, w, t) EW__(e, w, _##t)
#define EW__(e, w, t) e##w##t

#define pldd_assert(name, exp) \
  typedef int __assert_##name[((exp) != 0) - 1]


struct E(link_map)
{
  EW(Addr) l_addr;
  EW(Addr) l_name;
  EW(Addr) l_ld;
  EW(Addr) l_next;
  EW(Addr) l_prev;
  EW(Addr) l_real;
  Lmid_t l_ns;
  EW(Addr) l_libname;
};
#if CLASS == __ELF_NATIVE_CLASS
pldd_assert (l_addr, (offsetof (struct link_map, l_addr)
			== offsetof (struct E(link_map), l_addr)));
pldd_assert (l_name, (offsetof (struct link_map, l_name)
			== offsetof (struct E(link_map), l_name)));
pldd_assert (l_next, (offsetof (struct link_map, l_next)
			== offsetof (struct E(link_map), l_next)));
#endif


struct E(libname_list)
{
  EW(Addr) name;
  EW(Addr) next;
};
#if CLASS == __ELF_NATIVE_CLASS
pldd_assert (name, (offsetof (struct libname_list, name)
		      == offsetof (struct E(libname_list), name)));
pldd_assert (next, (offsetof (struct libname_list, next)
		      == offsetof (struct E(libname_list), next)));
#endif

struct E(r_debug)
{
  int r_version;
#if CLASS == 64
  int pad;
#endif
  EW(Addr) r_map;
};
#if CLASS == __ELF_NATIVE_CLASS
pldd_assert (r_version, (offsetof (struct r_debug, r_version)
			   == offsetof (struct E(r_debug), r_version)));
pldd_assert (r_map, (offsetof (struct r_debug, r_map)
		       == offsetof (struct E(r_debug), r_map)));
#endif


static int

E(find_maps) (pid_t pid, void *auxv, size_t auxv_size)
{
  EW(Addr) phdr = 0;
  unsigned int phnum = 0;
  unsigned int phent = 0;

  EW(auxv_t) *auxvXX = (EW(auxv_t) *) auxv;
  for (int i = 0; i < auxv_size / sizeof (EW(auxv_t)); ++i)
    switch (auxvXX[i].a_type)
      {
      case AT_PHDR:
	phdr = auxvXX[i].a_un.a_val;
	break;
      case AT_PHNUM:
	phnum = auxvXX[i].a_un.a_val;
	break;
      case AT_PHENT:
	phent = auxvXX[i].a_un.a_val;
	break;
      default:
	break;
      }

  if (phdr == 0 || phnum == 0 || phent == 0)
    error (EXIT_FAILURE, 0, gettext ("cannot find program header of process"));

  EW(Phdr) *p = alloca (phnum * phent);
  if (pread64 (memfd, p, phnum * phent, phdr) != phnum * phent)
    {
      error (0, 0, gettext ("cannot read program header"));
      return EXIT_FAILURE;
    }

  /* Determine the load offset.  We need this for interpreting the
     other program header entries so we do this in a separate loop.
     Fortunately it is the first time unless someone does something
     stupid when linking the application.  */
  EW(Addr) offset = 0;
  for (unsigned int i = 0; i < phnum; ++i)
    if (p[i].p_type == PT_PHDR)
      {
	offset = phdr - p[i].p_vaddr;
	break;
      }

  EW(Addr) list = 0;
  char *interp = NULL;
  for (unsigned int i = 0; i < phnum; ++i)
    if (p[i].p_type == PT_DYNAMIC)
      {
	EW(Dyn) *dyn = xmalloc (p[i].p_filesz);
	if (pread64 (memfd, dyn, p[i].p_filesz, offset + p[i].p_vaddr)
	    != p[i].p_filesz)
	  {
	    error (0, 0, gettext ("cannot read dynamic section"));
	    return EXIT_FAILURE;
	  }

	/* Search for the DT_DEBUG entry.  */
	for (unsigned int j = 0; j < p[i].p_filesz / sizeof (EW(Dyn)); ++j)
	  if (dyn[j].d_tag == DT_DEBUG && dyn[j].d_un.d_ptr != 0)
	    {
	      struct E(r_debug) r;
	      if (pread64 (memfd, &r, sizeof (r), dyn[j].d_un.d_ptr)
		  != sizeof (r))
		{
		  error (0, 0, gettext ("cannot read r_debug"));
		  return EXIT_FAILURE;
		}

	      if (r.r_map != 0)
		{
		  list = r.r_map;
		  break;
		}
	    }

	free (dyn);
	break;
      }
    else if (p[i].p_type == PT_INTERP)
      {
	interp = alloca (p[i].p_filesz);
	if (pread64 (memfd, interp, p[i].p_filesz, offset + p[i].p_vaddr)
	    != p[i].p_filesz)
	  {
	    error (0, 0, gettext ("cannot read program interpreter"));
	    return EXIT_FAILURE;
	  }
      }

  if (list == 0)
    {
      if (interp == NULL)
	{
	  // XXX check whether the executable itself is the loader
	  return EXIT_FAILURE;
	}

      // XXX perhaps try finding ld.so and _r_debug in it

      return EXIT_FAILURE;
    }

  /* Print the PID and program name first.  */
  printf ("%lu:\t%s\n", (unsigned long int) pid, exe);

  /* Iterate over the list of objects and print the information.  */
  size_t strsize = 256;
  char *str = alloca (strsize);
  do
    {
      struct E(link_map) m;
      if (pread64 (memfd, &m, sizeof (m), list) != sizeof (m))
	{
	  error (0, 0, gettext ("cannot read link map"));
	  return EXIT_FAILURE;
	}

      EW(Addr) name_offset = m.l_name;
    again:
      while (1)
	{
	  ssize_t n = pread64 (memfd, str, strsize, name_offset);
	  if (n == -1)
	    {
	      error (0, 0, gettext ("cannot read object name"));
	      return EXIT_FAILURE;
	    }

	  if (memchr (str, '\0', n) != NULL)
	    break;

	  str = extend_alloca (str, strsize, strsize * 2);
	}

      if (str[0] == '\0' && name_offset == m.l_name
	  && m.l_libname != 0)
	{
	  /* Try the l_libname element.  */
	  struct E(libname_list) ln;
	  if (pread64 (memfd, &ln, sizeof (ln), m.l_libname) == sizeof (ln))
	    {
	      name_offset = ln.name;
	      goto again;
	    }
	}

      /* Skip over the executable.  */
      if (str[0] != '\0')
	printf ("%s\n", str);

      list = m.l_next;
    }
  while (list != 0);

  return 0;
}


#undef CLASS
