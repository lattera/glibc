/* Read and display shared object profiling data.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <argp.h>
#include <dlfcn.h>
#include <elf.h>
#include <endian.h>
#include <error.h>
#include <fcntl.h>
#include <inttypes.h>
#include <libintl.h>
#include <link.h>
#include <locale.h>
#include <obstack.h>
#include <search.h>
#include <stab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/gmon.h>
#include <sys/gmon_out.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>

/* Undefine the following line line in the production version.  */
/* #define _NDEBUG 1 */
#include <assert.h>

/* Get libc version number.  */
#include "../version.h"

#define PACKAGE _libc_intl_domainname


#include <endian.h>
#if BYTE_ORDER == BIG_ENDIAN
#define byteorder ELFDATA2MSB
#define byteorder_name "big-endian"
#elif BYTE_ORDER == LITTLE_ENDIAN
#define byteorder ELFDATA2LSB
#define byteorder_name "little-endian"
#else
#error "Unknown BYTE_ORDER " BYTE_ORDER
#define byteorder ELFDATANONE
#endif


extern int __profile_frequency __P ((void));

/* Name and version of program.  */
static void print_version (FILE *stream, struct argp_state *state);
void (*argp_program_version_hook) (FILE *, struct argp_state *) = print_version;

#define OPT_COUNT_TOTAL	1
#define OPT_TEST 2

/* Definitions of arguments for argp functions.  */
static const struct argp_option options[] =
{
  { NULL, 0, NULL, 0, N_("Output selection:") },
  { "count-total", OPT_COUNT_TOTAL, NULL, 0,
    N_("print number of invocations for each function") },
  { "test", OPT_TEST, NULL, OPTION_HIDDEN, NULL },
  { NULL, 0, NULL, 0, NULL }
};

/* Short description of program.  */
static const char doc[] = N_("Read and display shared object profiling data");

/* Strings for arguments in help texts.  */
static const char args_doc[] = N_("SHOBJ [PROFDATA]");

/* Prototype for option handler.  */
static error_t parse_opt (int key, char *arg, struct argp_state *state);

/* Data structure to communicate with argp functions.  */
static struct argp argp =
{
  options, parse_opt, args_doc, doc, NULL, NULL
};


/* Operation modes.  */
static enum
{
  NONE = 0,
  COUNT_TOTAL
} mode;

/* If nonzero the total number of invocations of a function is emitted.  */
int count_total;

/* Nozero for testing.  */
int do_test;

/* Strcuture describing calls.  */
struct here_fromstruct
  {
    struct here_cg_arc_record volatile *here;
    uint16_t link;
  };

/* We define a special type to address the elements of the arc table.
   This is basically the `gmon_cg_arc_record' format but it includes
   the room for the tag and it uses real types.  */
struct here_cg_arc_record
  {
    uintptr_t from_pc;
    uintptr_t self_pc;
    uint32_t count;
  } __attribute__ ((packed));

/* Information about the stab debugging info.  This should be in a
   head but it is not.  */
#define STRDXOFF (0)
#define TYPEOFF (4)
#define OTHEROFF (5)
#define DESCOFF (6)
#define VALOFF (8)
#define STABSIZE (12)


struct known_symbol
{
  const char *name;
  uintptr_t addr;
  size_t size;
};


struct shobj
{
  const char *name;		/* User-provided name.  */

  struct link_map *map;
  const char *strtab;		/* String table of shared object.  */
  const char *soname;		/* Soname of shared object.  */

  uintptr_t lowpc;
  uintptr_t highpc;
  unsigned long int kcountsize;
  size_t expected_size;		/* Expected size of profiling file.  */
  size_t tossize;
  size_t fromssize;
  size_t fromlimit;
  unsigned int hashfraction;
  int s_scale;

  void *stab_map;
  size_t stab_mapsize;
  const char *stab;
  size_t stab_size;
  const char *stabstr;
  size_t stabstr_size;

  struct obstack ob_str;
  struct obstack ob_sym;
};


struct profdata
{
  void *addr;
  off_t size;

  char *hist;
  uint16_t *kcount;
  uint32_t narcs;		/* Number of arcs in toset.  */
  struct here_cg_arc_record *data;
  uint16_t *tos;
  struct here_fromstruct *froms;
};

/* Search tree for symbols.  */
void *symroot;
static const struct known_symbol **sortsym;
static size_t symidx;

/* Prototypes for local functions.  */
static struct shobj *load_shobj (const char *name);
static void unload_shobj (struct shobj *shobj);
static struct profdata *load_profdata (const char *name, struct shobj *shobj);
static void unload_profdata (struct profdata *profdata);
static void count_total_ticks (struct shobj *shobj, struct profdata *profdata);
static void read_symbols (struct shobj *shobj);


int
main (int argc, char *argv[])
{
  const char *shobj;
  const char *profdata;
  struct shobj *shobj_handle;
  struct profdata *profdata_handle;
  int remaining;

  setlocale (LC_ALL, "");

  /* Initialize the message catalog.  */
  textdomain (_libc_intl_domainname);

  /* Parse and process arguments.  */
  argp_parse (&argp, argc, argv, 0, &remaining, NULL);

  if (argc - remaining == 0 || argc - remaining > 2)
    {
      /* We need exactly two non-option parameter.  */
      argp_help (&argp, stdout, ARGP_HELP_SEE | ARGP_HELP_EXIT_ERR,
                 program_invocation_short_name);
      exit (1);
    }

  /* Get parameters.  */
  shobj = argv[remaining];
  if (argc - remaining == 2)
    profdata = argv[remaining + 1];
  else
    /* No filename for the profiling data given.  We will determine it
       from the soname of the shobj, later.  */
    profdata = NULL;

  /* First see whether we can load the shared object.  */
  shobj_handle = load_shobj (shobj);
  if (shobj_handle == NULL)
    exit (1);

  /* We can now determine the filename for the profiling data, if
     nececessary.  */
  if (profdata == NULL)
    {
      char *newp;

      if (shobj_handle->soname == NULL)
	{
	  unload_shobj (shobj_handle);

	  error (EXIT_FAILURE, 0, _("\
no filename for profiling data given and shared object `%s' has no soname"),
		 shobj);
	}

      newp = (char *) alloca (strlen (shobj_handle->soname)
			      + sizeof ".profile");
      stpcpy (stpcpy (newp, shobj_handle->soname), ".profile");
      profdata = newp;
    }

  /* Now see whether the profiling data file matches the given object.   */
  profdata_handle = load_profdata (profdata, shobj_handle);
  if (profdata_handle == NULL)
    {
      unload_shobj (shobj_handle);

      exit (1);
    }

  read_symbols (shobj_handle);

  /* Do some work.  */
  switch (mode)
    {
    case COUNT_TOTAL:
      count_total_ticks (shobj_handle, profdata_handle);
      break;
    case NONE:
      /* Do nothing.  */
      break;
    default:
      assert (! "Internal error");
    }

  /* Free the resources.  */
  unload_shobj (shobj_handle);
  unload_profdata (profdata_handle);

  return 0;
}


/* Handle program arguments.  */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  switch (key)
    {
    case OPT_COUNT_TOTAL:
      mode = COUNT_TOTAL;
      break;
    case OPT_TEST:
      do_test = 1;
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}


/* Print the version information.  */
static void
print_version (FILE *stream, struct argp_state *state)
{
  fprintf (stream, "sprof (GNU %s) %s\n", PACKAGE, VERSION);
  fprintf (stream, gettext ("\
Copyright (C) %s Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
"),
	   "1997, 1998");
  fprintf (stream, gettext ("Written by %s.\n"), "Ulrich Drepper");
}


/* Note that we must not use `dlopen' etc.  The shobj object must not
   be loaded for use.  */
static struct shobj *
load_shobj (const char *name)
{
  struct link_map *map = NULL;
  struct shobj *result;
  ElfW(Addr) mapstart = ~((ElfW(Addr)) 0);
  ElfW(Addr) mapend = 0;
  const ElfW(Phdr) *ph;
  size_t textsize;
  unsigned int log_hashfraction;
  ElfW(Ehdr) *ehdr;
  int fd;
  ElfW(Shdr) *shdr;
  void *ptr;
  size_t pagesize = getpagesize ();
  const char *shstrtab;
  int idx;
  ElfW(Shdr) *stab_entry;
  ElfW(Shdr) *stabstr_entry;

  /* Since we use dlopen() we must be prepared to work around the sometimes
     strange lookup rules for the shared objects.  If we have a file foo.so
     in the current directory and the user specfies foo.so on the command
     line (without specifying a directory) we should load the file in the
     current directory even if a normal dlopen() call would read the other
     file.  We do this by adding a directory portion to the name.  */
  if (strchr (name, '/') == NULL)
    {
      char *load_name = (char *) alloca (strlen (name) + 3);
      stpcpy (stpcpy (load_name, "./"), name);

      map = (struct link_map *) dlopen (load_name, RTLD_LAZY);
    }
  if (map == NULL)
    {
      map = (struct link_map *) dlopen (name, RTLD_LAZY);
      if (map == NULL)
	{
	  error (0, errno, _("failed to load shared object `%s'"), name);
	  return NULL;
	}
    }

  /* Prepare the result.  */
  result = (struct shobj *) calloc (1, sizeof (struct shobj));
  if (result == NULL)
    {
      error (0, errno, _("cannot create internal descriptors"));
      dlclose (map);
      return NULL;
    }
  result->name = name;
  result->map = map;

  /* Compute the size of the sections which contain program code.
     This must match the code in dl-profile.c (_dl_start_profile).  */
  for (ph = map->l_phdr; ph < &map->l_phdr[map->l_phnum]; ++ph)
    if (ph->p_type == PT_LOAD && (ph->p_flags & PF_X))
      {
	ElfW(Addr) start = (ph->p_vaddr & ~(_dl_pagesize - 1));
	ElfW(Addr) end = ((ph->p_vaddr + ph->p_memsz + _dl_pagesize - 1)
			  & ~(_dl_pagesize - 1));

	if (start < mapstart)
	  mapstart = start;
	if (end > mapend)
	  mapend = end;
      }

  result->lowpc = ROUNDDOWN ((uintptr_t) (mapstart + map->l_addr),
			     HISTFRACTION * sizeof (HISTCOUNTER));
  result->highpc = ROUNDUP ((uintptr_t) (mapend + map->l_addr),
			    HISTFRACTION * sizeof (HISTCOUNTER));
  if (do_test)
    printf ("load addr: %0#*" PRIxPTR "\n"
	    "lower bound PC: %0#*" PRIxPTR "\n"
	    "upper bound PC: %0#*" PRIxPTR "\n",
	    __ELF_NATIVE_CLASS == 32 ? 10 : 18, map->l_addr,
	    __ELF_NATIVE_CLASS == 32 ? 10 : 18, result->lowpc,
	    __ELF_NATIVE_CLASS == 32 ? 10 : 18, result->highpc);

  textsize = result->highpc - result->lowpc;
  result->kcountsize = textsize / HISTFRACTION;
  result->hashfraction = HASHFRACTION;
  if ((HASHFRACTION & (HASHFRACTION - 1)) == 0)
    /* If HASHFRACTION is a power of two, mcount can use shifting
       instead of integer division.  Precompute shift amount.  */
    log_hashfraction = __builtin_ffs (result->hashfraction
				      * sizeof (struct here_fromstruct)) - 1;
  else
    log_hashfraction = -1;
  if (do_test)
    printf ("hashfraction = %d\ndivider = %d\n",
	    result->hashfraction,
	    result->hashfraction * sizeof (struct here_fromstruct));
  result->tossize = textsize / HASHFRACTION;
  result->fromlimit = textsize * ARCDENSITY / 100;
  if (result->fromlimit < MINARCS)
    result->fromlimit = MINARCS;
  if (result->fromlimit > MAXARCS)
    result->fromlimit = MAXARCS;
  result->fromssize = result->fromlimit * sizeof (struct here_fromstruct);

  result->expected_size = (sizeof (struct gmon_hdr)
			   + 4 + sizeof (struct gmon_hist_hdr)
			   + result->kcountsize
			   + 4 + 4
			   + (result->fromssize
			      * sizeof (struct here_cg_arc_record)));

  if (do_test)
    {
#define SCALE_1_TO_1	0x10000L

      printf ("expected size: %Zd\n", result->expected_size);

      if (result->kcountsize < result->highpc - result->lowpc)
	{
	  size_t range = result->highpc - result->lowpc;
	  size_t quot = range / result->kcountsize;

	  if (quot >= SCALE_1_TO_1)
	    result->s_scale = 1;
	  else if (quot >= SCALE_1_TO_1 / 256)
	    result->s_scale = SCALE_1_TO_1 / quot;
	  else if (range > ULONG_MAX / 256)
	    result->s_scale = ((SCALE_1_TO_1 * 256)
			       / (range / (result->kcountsize / 256)));
	  else
	    result->s_scale = ((SCALE_1_TO_1 * 256)
			       / ((range * 256) / result->kcountsize));
	}
      else
	result->s_scale = SCALE_1_TO_1;

      printf ("s_scale: %d\n", result->s_scale);
    }

  /* Determine the string table.  */
  if (map->l_info[DT_STRTAB] == NULL)
    result->strtab = NULL;
  else
    result->strtab = (const char *) (map->l_addr
				     + map->l_info[DT_STRTAB]->d_un.d_ptr);
  if (do_test)
    printf ("string table: %p\n", result->strtab);

  /* Determine the soname.  */
  if (map->l_info[DT_SONAME] == NULL)
    result->soname = NULL;
  else
    result->soname = result->strtab + map->l_info[DT_SONAME]->d_un.d_val;
  if (do_test)
    printf ("soname: %s\n", result->soname);

  /* Now the hard part, we have to load the debugging data.  For now
     we support stabs only.

     First load the section header table.  */
  ehdr = (ElfW(Ehdr) *) map->l_addr;

  /* Make sure we are on the right party.  */
  if (ehdr->e_shentsize != sizeof (ElfW(Shdr)))
    abort ();

  /* And we need the shared object file descriptor again.  */
  fd = open (map->l_name, O_RDONLY);
  if (fd == -1)
    /* Dooh, this really shouldn't happen.  We know the file is available.  */
    error (EXIT_FAILURE, errno, _("Reopening shared object `%s' failed"));

  /* Now map the section header.  */
  ptr = mmap (NULL, (ehdr->e_phnum * sizeof (ElfW(Shdr))
		     + (ehdr->e_shoff & (pagesize - 1))), PROT_READ,
	      MAP_SHARED|MAP_FILE, fd, ehdr->e_shoff & ~(pagesize - 1));
  if (ptr == MAP_FAILED)
    error (EXIT_FAILURE, errno, _("mapping of section headers failed"));
  shdr = (ElfW(Shdr) *) ((char *) ptr + (ehdr->e_shoff & (pagesize - 1)));

  /* Get the section header string table.  */
  ptr = mmap (NULL, (shdr[ehdr->e_shstrndx].sh_size
		     + (shdr[ehdr->e_shstrndx].sh_offset & (pagesize - 1))),
	      PROT_READ, MAP_SHARED|MAP_FILE, fd,
	      shdr[ehdr->e_shstrndx].sh_offset & ~(pagesize - 1));
  if (ptr == MAP_FAILED)
    error (EXIT_FAILURE, errno,
	   _("mapping of section header string table failed"));
  shstrtab = ((const char *) ptr
	      + (shdr[ehdr->e_shstrndx].sh_offset & (pagesize - 1)));

  /* Search for the ".stab" and ".stabstr" section (and ".rel.stab" ?).  */
  stab_entry = NULL;
  stabstr_entry = NULL;
  for (idx = 0; idx < ehdr->e_shnum; ++idx)
    /* We only have to look for sections which are not loaded.  */
    if (shdr[idx].sh_addr == 0)
      {
	if (strcmp (shstrtab + shdr[idx].sh_name, ".stab") == 0)
	  stab_entry = &shdr[idx];
	else if (strcmp (shstrtab + shdr[idx].sh_name, ".stabstr") == 0)
	  stabstr_entry = &shdr[idx];
      }

  /* We don't need the sectin header string table anymore.  */
  munmap (ptr, (shdr[ehdr->e_shstrndx].sh_size
		+ (shdr[ehdr->e_shstrndx].sh_offset & (pagesize - 1))));

  if (stab_entry == NULL || stabstr_entry == NULL)
    {
      fprintf (stderr, _("\
*** The file `%s' is stripped: no detailed analysis possible\n"),
	      name);
      result->stab = NULL;
      result->stabstr = NULL;
    }
  else
    {
      if (stab_entry->sh_offset + stab_entry->sh_size
	  != stabstr_entry->sh_offset)
	abort ();
      if (stab_entry->sh_size % STABSIZE != 0)
	abort ();

      result->stab_map = mmap (NULL, (stab_entry->sh_size
				      + stabstr_entry->sh_size
				      + (stab_entry->sh_offset
					 & (pagesize - 1))),
			       PROT_READ, MAP_SHARED|MAP_FILE, fd,
			       stab_entry->sh_offset & ~(pagesize - 1));
      if (result->stab_map == NULL)
	error (EXIT_FAILURE, errno, _("failed to load stab data:"));

      result->stab = ((const char *) result->stab_map
		      + (stab_entry->sh_offset & (pagesize - 1)));
      result->stab_size = stab_entry->sh_size;
      result->stabstr = result->stab + stab_entry->sh_size;
      result->stabstr_size = stabstr_entry->sh_size;
      result->stab_mapsize = (stab_entry->sh_size + stabstr_entry->sh_size
			      + (stab_entry->sh_offset & (pagesize - 1)));
    }

  /* Now we also don't need the sectio header table anymore.  */
  munmap ((char *) shdr - (ehdr->e_shoff & (pagesize - 1)),
	  (ehdr->e_phnum * sizeof (ElfW(Shdr))
	   + (ehdr->e_shoff & (pagesize - 1))));

  /* Free the descriptor for the shared object.  */
  close (fd);

  return result;
}


static void
unload_shobj (struct shobj *shobj)
{
  munmap (shobj->stab_map, shobj->stab_mapsize);
  dlclose (shobj->map);
}


static struct profdata *
load_profdata (const char *name, struct shobj *shobj)
{
  struct profdata *result;
  int fd;
  struct stat st;
  void *addr;
  struct gmon_hdr gmon_hdr;
  struct gmon_hist_hdr hist_hdr;
  uint32_t *narcsp;
  size_t fromlimit;
  struct here_cg_arc_record *data;
  struct here_fromstruct *froms;
  uint16_t *tos;
  size_t fromidx;
  size_t idx;

  fd = open (name, O_RDONLY);
  if (fd == -1)
    {
      char *ext_name;

      if (errno != ENOENT || strchr (name, '/') != NULL)
	/* The file exists but we are not allowed to read it or the
	   file does not exist and the name includes a path
	   specification..  */
	return NULL;

      /* A file with the given name does not exist in the current
	 directory, try it in the default location where the profiling
	 files are created.  */
      ext_name = (char *) alloca (strlen (name) + sizeof "/var/tmp/");
      stpcpy (stpcpy (ext_name, "/var/tmp/"), name);
      name = ext_name;

      fd = open (ext_name, O_RDONLY);
      if (fd == -1)
	{
	  /* Even this file does not exist.  */
	  error (0, errno, _("cannot load profiling data"));
	  return NULL;
	}
    }

  /* We have found the file, now make sure it is the right one for the
     data file.  */
  if (fstat (fd, &st) < 0)
    {
      error (0, errno, _("while stat'ing profiling data file"));
      close (fd);
      return NULL;
    }

  if (st.st_size != shobj->expected_size)
    {
      error (0, 0, _("profiling data file `%s' does match shared object `%s'"),
	     name, shobj->name);
      close (fd);
      return NULL;
    }

  /* The data file is most probably the right one for our shared
     object.  Map it now.  */
  addr = mmap (NULL, st.st_size, PROT_READ, MAP_SHARED|MAP_FILE, fd, 0);
  if (addr == MAP_FAILED)
    {
      error (0, errno, _("failed to mmap the profiling data file"));
      close (fd);
      return NULL;
    }

  /* We don't need the file desriptor anymore.  */
  if (close (fd) < 0)
    {
      error (0, errno, _("error while closing the profiling data file"));
      munmap (addr, st.st_size);
      return NULL;
    }

  /* Prepare the result.  */
  result = (struct profdata *) calloc (1, sizeof (struct profdata));
  if (result == NULL)
    {
      error (0, errno, _("cannot create internal descriptor"));
      munmap (addr, st.st_size);
      return NULL;
    }

  /* Store the address and size so that we can later free the resources.  */
  result->addr = addr;
  result->size = st.st_size;

  /* Pointer to data after the header.  */
  result->hist = (char *) ((struct gmon_hdr *) addr + 1);
  result->kcount = (uint16_t *) ((char *) result->hist + sizeof (uint32_t)
				 + sizeof (struct gmon_hist_hdr));

  /* Compute pointer to array of the arc information.  */
  narcsp = (uint32_t *) ((char *) result->kcount + shobj->kcountsize
			 + sizeof (uint32_t));
  result->narcs = *narcsp;
  result->data = (struct here_cg_arc_record *) ((char *) narcsp
						+ sizeof (uint32_t));

  /* Create the gmon_hdr we expect or write.  */
  memset (&gmon_hdr, '\0', sizeof (struct gmon_hdr));
  memcpy (&gmon_hdr.cookie[0], GMON_MAGIC, sizeof (gmon_hdr.cookie));
  *(int32_t *) gmon_hdr.version = GMON_SHOBJ_VERSION;

  /* Create the hist_hdr we expect or write.  */
  *(char **) hist_hdr.low_pc = (char *) shobj->lowpc - shobj->map->l_addr;
  *(char **) hist_hdr.high_pc = (char *) shobj->highpc - shobj->map->l_addr;
  if (do_test)
    printf ("low_pc = %p\nhigh_pc = %p\n",
	    hist_hdr.low_pc, hist_hdr.high_pc);
  *(int32_t *) hist_hdr.hist_size = shobj->kcountsize / sizeof (HISTCOUNTER);
  *(int32_t *) hist_hdr.prof_rate = __profile_frequency ();
  strncpy (hist_hdr.dimen, "seconds", sizeof (hist_hdr.dimen));
  hist_hdr.dimen_abbrev = 's';

  /* Test whether the header of the profiling data is ok.  */
  if (memcmp (addr, &gmon_hdr, sizeof (struct gmon_hdr)) != 0
      || *(uint32_t *) result->hist != GMON_TAG_TIME_HIST
      || memcmp (result->hist + sizeof (uint32_t), &hist_hdr,
		 sizeof (struct gmon_hist_hdr)) != 0
      || narcsp[-1] != GMON_TAG_CG_ARC)
    {
      free (result);
      error (0, 0, _("`%s' is no correct profile data file for `%s'"),
	     name, shobj->name);
      munmap (addr, st.st_size);
      return NULL;
    }

  /* We are pretty sure now that this is a correct input file.  Set up
     the remaining information in the result structure and return.  */
  result->tos = (uint16_t *) calloc (shobj->tossize + shobj->fromssize, 1);
  if (result->tos == NULL)
    {
      error (0, errno, _("cannot create internal descriptor"));
      munmap (addr, st.st_size);
      free (result);
      return NULL;
    }

  result->froms = (struct here_fromstruct *) ((char *) result->tos
					      + shobj->tossize);
  fromidx = 0;

  /* Now we have to process all the arc count entries.  */
  fromlimit = shobj->fromlimit;
  data = result->data;
  froms = result->froms;
  tos = result->tos;
  for (idx = 0; idx < MIN (*narcsp, fromlimit); ++idx)
    {
      size_t to_index;
      size_t newfromidx;
      to_index = (data[idx].self_pc / (shobj->hashfraction * sizeof (*tos)));
      newfromidx = fromidx++;
      froms[newfromidx].here = &data[idx];
      froms[newfromidx].link = tos[to_index];
      tos[to_index] = newfromidx;
    }

  return result;
}


static void
unload_profdata (struct profdata *profdata)
{
  free (profdata->tos);
  munmap (profdata->addr, profdata->size);
  free (profdata);
}


static void
count_total_ticks (struct shobj *shobj, struct profdata *profdata)
{
  volatile uint16_t *kcount = profdata->kcount;
  uint64_t sum = 0;
  size_t idx;
  size_t factor = 2 * (65536 / shobj->s_scale);

  for (idx = shobj->kcountsize / sizeof (*kcount); idx > 0; )
    {
      --idx;
      if (kcount[idx] != 0)
	{
	  size_t n;

	  for (n = 0; n < symidx; ++n)
	    if (sortsym[n]->addr <= factor * idx
		&& sortsym[n]->addr + sortsym[n]->size > factor * idx)
	      break;

	  if (n < symidx)
	    printf ("idx = %d, count = %d, name = %s\n", idx, kcount[idx],
		    sortsym[n]->name);
	  else
	    printf ("idx = %d, N/A\n", idx);
	}
      sum += kcount[idx];
    }

  printf ("total ticks: %10" PRId64 "\n", sum);
}


static int
symorder (const void *o1, const void *o2)
{
  const struct known_symbol *p1 = (struct known_symbol *) o1;
  const struct known_symbol *p2 = (struct known_symbol *) o2;

  return p1->addr - p2->addr;
}


static void
printsym (const void *node, VISIT value, int level)
{
  if (value == leaf || value == postorder)
    {
      const struct known_symbol *sym = *(const struct known_symbol **) node;

      printf ("Name: %30s, Start: %6x, Len: %5d\n",
	      sym->name, sym->addr, sym->size);

      sortsym[symidx++] = sym;
    }
}


static void
read_symbols (struct shobj *shobj)
{
  void *load_addr = (void *) shobj->map->l_addr;
  int n = 0;
  int idx;
  const char *last_name = NULL;
  uintptr_t last_addr = 0;

  /* Initialize the obstacks.  */
#define obstack_chunk_alloc malloc
#define obstack_chunk_free free
  obstack_init (&shobj->ob_str);
  obstack_init (&shobj->ob_sym);

  /* Process the stabs.  */
  for (idx = 0; idx < shobj->stab_size; idx += 12)
    if (*(shobj->stab + idx + TYPEOFF) == N_FUN)
      {
	const char *str = (shobj->stabstr
			   + *((uint32_t *) (shobj->stab + idx + STRDXOFF)));

	if (*str != '\0')
	  {
	    last_name = str;
	    last_addr = *((uint32_t *) (shobj->stab + idx + VALOFF));
	  }
	else
	  {
	    const char *endp;
	    char *name0;
	    struct known_symbol *newsym;

	    if (last_name == NULL)
	      abort ();

	    endp = strchr (last_name, ':');

	    name0 = (char *) obstack_copy0 (&shobj->ob_str, last_name,
					    endp - last_name);
	    if (name0 != NULL)
	      newsym =
		(struct known_symbol *) obstack_alloc (&shobj->ob_sym,
						       sizeof (*newsym));
	    else
	      /* Keep the stupid compiler happy.  */
	      newsym = NULL;
	    if (name0 == NULL || newsym == NULL)
	      error (EXIT_FAILURE, errno, _("cannot allocate symbol data"));

	    newsym->name = name0;
	    newsym->addr = last_addr;
	    newsym->size = *((uint32_t *) (shobj->stab + idx + VALOFF));

	    tsearch (newsym, &symroot, symorder);
	    ++n;

	    last_name = NULL;
	    last_addr = 0;
	  }
      }

  if (shobj->stab == NULL)
    {
      /* Blarg, the binary is stripped.  We have to rely on the
	 information contained in the dynamic section of the object.  */
      const ElfW(Sym) *symtab = (load_addr
				 + shobj->map->l_info[DT_SYMTAB]->d_un.d_ptr);
      const char *strtab = (load_addr
			    + shobj->map->l_info[DT_STRTAB]->d_un.d_ptr);

      /* We assume that the string table follows the symbol table,
	 because there is no way in ELF to know the size of the
	 dynamic symbol table!!  */
      while ((void *) symtab < (void *) strtab)
	{
	  if (/*(ELFW(ST_TYPE)(symtab->st_info) == STT_FUNC
		|| ELFW(ST_TYPE)(symtab->st_info) == STT_NOTYPE)
		&&*/ symtab->st_size != 0)
	    {
	      struct known_symbol *newsym;

	      newsym =
		(struct known_symbol *) obstack_alloc (&shobj->ob_sym,
						       sizeof (*newsym));
	      if (newsym == NULL)
		error (EXIT_FAILURE, errno, _("cannot allocate symbol data"));

	      newsym->name = &strtab[symtab->st_name];
	      newsym->addr = symtab->st_value;
	      newsym->size = symtab->st_size;

	      tsearch (newsym, &symroot, symorder);
	      ++n;
	    }
	}

      ++symtab;
    }

  sortsym = malloc (n * sizeof (struct known_symbol *));
  if (sortsym == NULL)
    abort ();

  twalk (symroot, printsym);
}
