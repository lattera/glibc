/* Map in a shared object's segments from the file.
   Copyright (C) 1995,96,97,98,99,2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <libintl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ldsodefs.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "dynamic-link.h"
#include <stdio-common/_itoa.h>

#include <dl-dst.h>

/* On some systems, no flag bits are given to specify file mapping.  */
#ifndef MAP_FILE
# define MAP_FILE	0
#endif

/* The right way to map in the shared library files is MAP_COPY, which
   makes a virtual copy of the data at the time of the mmap call; this
   guarantees the mapped pages will be consistent even if the file is
   overwritten.  Some losing VM systems like Linux's lack MAP_COPY.  All we
   get is MAP_PRIVATE, which copies each page when it is modified; this
   means if the file is overwritten, we may at some point get some pages
   from the new version after starting with pages from the old version.  */
#ifndef MAP_COPY
# define MAP_COPY	MAP_PRIVATE
#endif

/* Some systems link their relocatable objects for another base address
   than 0.  We want to know the base address for these such that we can
   subtract this address from the segment addresses during mapping.
   This results in a more efficient address space usage.  Defaults to
   zero for almost all systems.  */
#ifndef MAP_BASE_ADDR
# define MAP_BASE_ADDR(l)	0
#endif


#include <endian.h>
#if BYTE_ORDER == BIG_ENDIAN
# define byteorder ELFDATA2MSB
#elif BYTE_ORDER == LITTLE_ENDIAN
# define byteorder ELFDATA2LSB
#else
# error "Unknown BYTE_ORDER " BYTE_ORDER
# define byteorder ELFDATANONE
#endif

#define STRING(x) __STRING (x)

#ifdef MAP_ANON
/* The fd is not examined when using MAP_ANON.  */
# define ANONFD -1
#else
int _dl_zerofd = -1;
# define ANONFD _dl_zerofd
#endif

/* Handle situations where we have a preferred location in memory for
   the shared objects.  */
#ifdef ELF_PREFERRED_ADDRESS_DATA
ELF_PREFERRED_ADDRESS_DATA;
#endif
#ifndef ELF_PREFERRED_ADDRESS
# define ELF_PREFERRED_ADDRESS(loader, maplength, mapstartpref) (mapstartpref)
#endif
#ifndef ELF_FIXED_ADDRESS
# define ELF_FIXED_ADDRESS(loader, mapstart) ((void) 0)
#endif

/* Type for the buffer we put the ELF header and hopefully the program
   header.  This buffer does not really have to be too large.  In most
   cases the program header follows the ELF header directly.  If this
   is not the case all bets are off and we can make the header arbitrarily
   large and still won't get it read.  This means the only question is
   how large are the ELF and program header combined.  The ELF header
   in 64-bit files is 56 bytes long.  Each program header entry is again
   56 bytes long.  I.e., even with a file which has 17 program header
   entries we only have to read 1kB.  And 17 program header entries is
   plenty, normal files have < 10.  If this heuristic should really fail
   for some file the code in `_dl_map_object_from_fd' knows how to
   recover.  */
struct filebuf
{
  ssize_t len;
  char buf[1024];
};

size_t _dl_pagesize;

int _dl_clktck;

extern const char *_dl_platform;
extern size_t _dl_platformlen;

/* This is the decomposed LD_LIBRARY_PATH search path.  */
static struct r_search_path_struct env_path_list;

/* List of the hardware capabilities we might end up using.  */
static const struct r_strlenpair *capstr;
static size_t ncapstr;
static size_t max_capstrlen;

const unsigned char _dl_pf_to_prot[8] =
{
  [0] = PROT_NONE,
  [PF_R] = PROT_READ,
  [PF_W] = PROT_WRITE,
  [PF_R | PF_W] = PROT_READ | PROT_WRITE,
  [PF_X] = PROT_EXEC,
  [PF_R | PF_X] = PROT_READ | PROT_EXEC,
  [PF_W | PF_X] = PROT_WRITE | PROT_EXEC,
  [PF_R | PF_W | PF_X] = PROT_READ | PROT_WRITE | PROT_EXEC
};


/* Get the generated information about the trusted directories.  */
#include "trusted-dirs.h"

static const char system_dirs[] = SYSTEM_DIRS;
static const size_t system_dirs_len[] =
{
  SYSTEM_DIRS_LEN
};
#define nsystem_dirs_len \
  (sizeof (system_dirs_len) / sizeof (system_dirs_len[0]))


/* Local version of `strdup' function.  */
static inline char *
local_strdup (const char *s)
{
  size_t len = strlen (s) + 1;
  void *new = malloc (len);

  if (new == NULL)
    return NULL;

  return (char *) memcpy (new, s, len);
}


size_t
_dl_dst_count (const char *name, int is_path)
{
  const char *const start = name;
  size_t cnt = 0;

  do
    {
      size_t len = 1;

      /* $ORIGIN is not expanded for SUID/GUID programs.

	 Note that it is no bug that the strings in the first two `strncmp'
	 calls are longer than the sequence which is actually tested.  */
      if ((((strncmp (&name[1], "ORIGIN}", 6) == 0
	     && (!__libc_enable_secure
		 || ((name[7] == '\0' || (is_path && name[7] == ':'))
		     && (name == start || (is_path && name[-1] == ':'))))
	     && (len = 7) != 0)
	    || (strncmp (&name[1], "PLATFORM}", 8) == 0 && (len = 9) != 0))
	   && (name[len] == '\0' || name[len] == '/'
	       || (is_path && name[len] == ':')))
	  || (name[1] == '{'
	      && ((strncmp (&name[2], "ORIGIN}", 7) == 0
		   && (!__libc_enable_secure
		       || ((name[9] == '\0' || (is_path && name[9] == ':'))
			   && (name == start || (is_path && name[-1] == ':'))))
		   && (len = 9) != 0)
		  || (strncmp (&name[2], "PLATFORM}", 9) == 0
		      && (len = 11) != 0))))
	++cnt;

      name = strchr (name + len, '$');
    }
  while (name != NULL);

  return cnt;
}


char *
_dl_dst_substitute (struct link_map *l, const char *name, char *result,
		    int is_path)
{
  const char *const start = name;
  char *last_elem, *wp;

  /* Now fill the result path.  While copying over the string we keep
     track of the start of the last path element.  When we come accross
     a DST we copy over the value or (if the value is not available)
     leave the entire path element out.  */
  last_elem = wp = result;

  do
    {
      if (*name == '$')
	{
	  const char *repl;
	  size_t len;

	  /* Note that it is no bug that the strings in the first two `strncmp'
	     calls are longer than the sequence which is actually tested.  */
	  if ((((strncmp (&name[1], "ORIGIN}", 6) == 0 && (len = 7) != 0)
		|| (strncmp (&name[1], "PLATFORM}", 8) == 0 && (len = 9) != 0))
	       && (name[len] == '\0' || name[len] == '/'
		   || (is_path && name[len] == ':')))
	      || (name[1] == '{'
		  && ((strncmp (&name[2], "ORIGIN}", 7) == 0 && (len = 9) != 0)
		      || (strncmp (&name[2], "PLATFORM}", 9) == 0
			  && (len = 11) != 0))))
	    {
	      repl = ((len == 7 || name[2] == 'O')
		      ? (__libc_enable_secure
			 && ((name[len] != '\0'
			      && (!is_path || name[len] != ':'))
			     || (name != start
				 && (!is_path || name[-1] != ':')))
			 ? NULL : l->l_origin)
		      : _dl_platform);

	      if (repl != NULL && repl != (const char *) -1)
		{
		  wp = __stpcpy (wp, repl);
		  name += len;
		}
	      else
		{
		  /* We cannot use this path element, the value of the
		     replacement is unknown.  */
		  wp = last_elem;
		  name += len;
		  while (*name != '\0' && (!is_path || *name != ':'))
		    ++name;
		}
	    }
	  else
	    /* No DST we recognize.  */
	    *wp++ = *name++;
	}
      else if (is_path && *name == ':')
	{
	  *wp++ = *name++;
	  last_elem = wp;
	}
      else
	*wp++ = *name++;
    }
  while (*name != '\0');

  *wp = '\0';

  return result;
}


/* Return copy of argument with all recognized dynamic string tokens
   ($ORIGIN and $PLATFORM for now) replaced.  On some platforms it
   might not be possible to determine the path from which the object
   belonging to the map is loaded.  In this case the path element
   containing $ORIGIN is left out.  */
static char *
expand_dynamic_string_token (struct link_map *l, const char *s)
{
  /* We make two runs over the string.  First we determine how large the
     resulting string is and then we copy it over.  Since this is now
     frequently executed operation we are looking here not for performance
     but rather for code size.  */
  size_t cnt;
  size_t total;
  char *result;

  /* Determine the number of DST elements.  */
  cnt = DL_DST_COUNT (s, 1);

  /* If we do not have to replace anything simply copy the string.  */
  if (cnt == 0)
    return local_strdup (s);

  /* Determine the length of the substituted string.  */
  total = DL_DST_REQUIRED (l, s, strlen (s), cnt);

  /* Allocate the necessary memory.  */
  result = (char *) malloc (total + 1);
  if (result == NULL)
    return NULL;

  return DL_DST_SUBSTITUTE (l, s, result, 1);
}


/* Add `name' to the list of names for a particular shared object.
   `name' is expected to have been allocated with malloc and will
   be freed if the shared object already has this name.
   Returns false if the object already had this name.  */
static void
internal_function
add_name_to_object (struct link_map *l, const char *name)
{
  struct libname_list *lnp, *lastp;
  struct libname_list *newname;
  size_t name_len;

  lastp = NULL;
  for (lnp = l->l_libname; lnp != NULL; lastp = lnp, lnp = lnp->next)
    if (strcmp (name, lnp->name) == 0)
      return;

  name_len = strlen (name) + 1;
  newname = (struct libname_list *) malloc (sizeof *newname + name_len);
  if (newname == NULL)
    {
      /* No more memory.  */
      _dl_signal_error (ENOMEM, name, N_("cannot allocate name record"));
      return;
    }
  /* The object should have a libname set from _dl_new_object.  */
  assert (lastp != NULL);

  newname->name = memcpy (newname + 1, name, name_len);
  newname->next = NULL;
  newname->dont_free = 0;
  lastp->next = newname;
}

/* All known directories in sorted order.  */
struct r_search_path_elem *_dl_all_dirs;

/* All directories after startup.  */
struct r_search_path_elem *_dl_init_all_dirs;

/* Standard search directories.  */
static struct r_search_path_struct rtld_search_dirs;

static size_t max_dirnamelen;

static inline struct r_search_path_elem **
fillin_rpath (char *rpath, struct r_search_path_elem **result, const char *sep,
	      int check_trusted, const char *what, const char *where)
{
  char *cp;
  size_t nelems = 0;

  while ((cp = __strsep (&rpath, sep)) != NULL)
    {
      struct r_search_path_elem *dirp;
      size_t len = strlen (cp);

      /* `strsep' can pass an empty string.  This has to be
         interpreted as `use the current directory'. */
      if (len == 0)
	{
	  static const char curwd[] = "./";
	  cp = (char *) curwd;
	}

      /* Remove trailing slashes (except for "/").  */
      while (len > 1 && cp[len - 1] == '/')
	--len;

      /* Now add one if there is none so far.  */
      if (len > 0 && cp[len - 1] != '/')
	cp[len++] = '/';

      /* See if this directory is already known.  */
      for (dirp = _dl_all_dirs; dirp != NULL; dirp = dirp->next)
	if (dirp->dirnamelen == len && memcmp (cp, dirp->dirname, len) == 0)
	  break;

      if (dirp != NULL)
	{
	  /* It is available, see whether it's on our own list.  */
	  size_t cnt;
	  for (cnt = 0; cnt < nelems; ++cnt)
	    if (result[cnt] == dirp)
	      break;

	  if (cnt == nelems)
	    result[nelems++] = dirp;
	}
      else
	{
	  size_t cnt;
	  enum r_dir_status init_val;
	  size_t where_len = where ? strlen (where) + 1 : 0;

	  /* It's a new directory.  Create an entry and add it.  */
	  dirp = (struct r_search_path_elem *)
	    malloc (sizeof (*dirp) + ncapstr * sizeof (enum r_dir_status)
		    + where_len + len + 1);
	  if (dirp == NULL)
	    _dl_signal_error (ENOMEM, NULL,
			      N_("cannot create cache for search path"));

	  dirp->dirname = ((char *) dirp + sizeof (*dirp)
			   + ncapstr * sizeof (enum r_dir_status));
	  memcpy ((char *) dirp->dirname, cp, len + 1);
	  dirp->dirnamelen = len;

	  if (len > max_dirnamelen)
	    max_dirnamelen = len;

	  /* Make sure we don't use untrusted directories if we run SUID.  */
	  if (__builtin_expect (check_trusted, 0))
	    {
	      const char *trun = system_dirs;
	      size_t idx;

	      /* By default we don't trust anything.  */
	      init_val = nonexisting;

	      /* All trusted directories must be complete names.  */
	      if (cp[0] == '/')
		{
		  for (idx = 0; idx < nsystem_dirs_len; ++idx)
		    {
		      if (len == system_dirs_len[idx]
			  && memcmp (trun, cp, len) == 0)
			/* Found it.  */
			break;

		      trun += system_dirs_len[idx] + 1;
		    }

		  if (idx < nsystem_dirs_len)
		    /* It's a trusted directory so allow checking for it.  */
		    init_val = unknown;
		}
	    }
	  else
	    /* We don't have to check for trusted directories and can
	       accept everything.  We have to make sure all the
	       relative directories are never ignored.  The current
	       directory might change and all our saved information
	       would be void.  */
	    init_val = cp[0] != '/' ? existing : unknown;

	  for (cnt = 0; cnt < ncapstr; ++cnt)
	    dirp->status[cnt] = init_val;

	  dirp->what = what;
	  if (__builtin_expect (where != NULL, 1))
	    dirp->where = memcpy ((char *) dirp + sizeof (*dirp) + len + 1
				  + ncapstr * sizeof (enum r_dir_status),
				  where, where_len);
	  else
	    dirp->where = NULL;

	  dirp->next = _dl_all_dirs;
	  _dl_all_dirs = dirp;

	  /* Put it in the result array.  */
	  result[nelems++] = dirp;
	}
    }

  /* Terminate the array.  */
  result[nelems] = NULL;

  return result;
}


static void
internal_function
decompose_rpath (struct r_search_path_struct *sps,
		 const char *rpath, struct link_map *l, const char *what)
{
  /* Make a copy we can work with.  */
  const char *where = l->l_name;
  char *copy;
  char *cp;
  struct r_search_path_elem **result;
  size_t nelems;

  /* First see whether we must forget the RUNPATH and RPATH from this
     object.  */
  if (__builtin_expect (_dl_inhibit_rpath != NULL, 0) && !__libc_enable_secure)
    {
      const char *found = strstr (_dl_inhibit_rpath, where);
      if (found != NULL)
	{
	  size_t len = strlen (where);
	  if ((found == _dl_inhibit_rpath || found[-1] == ':')
	      && (found[len] == '\0' || found[len] == ':'))
	    {
	      /* This object is on the list of objects for which the
		 RUNPATH and RPATH must not be used.  */
	      result = (struct r_search_path_elem **)
		malloc (sizeof (*result));
	      if (result == NULL)
		_dl_signal_error (ENOMEM, NULL,
				  N_("cannot create cache for search path"));
	      result[0] = NULL;

	      sps->dirs = result;
	      sps->malloced = 1;

	      return;
	    }
	}
    }

  /* Make a writable copy.  At the same time expand possible dynamic
     string tokens.  */
  copy = expand_dynamic_string_token (l, rpath);
  if (copy == NULL)
    _dl_signal_error (ENOMEM, NULL, N_("cannot create RUNPATH/RPATH copy"));

  /* Count the number of necessary elements in the result array.  */
  nelems = 0;
  for (cp = copy; *cp != '\0'; ++cp)
    if (*cp == ':')
      ++nelems;

  /* Allocate room for the result.  NELEMS + 1 is an upper limit for the
     number of necessary entries.  */
  result = (struct r_search_path_elem **) malloc ((nelems + 1 + 1)
						  * sizeof (*result));
  if (result == NULL)
    _dl_signal_error (ENOMEM, NULL, N_("cannot create cache for search path"));

  fillin_rpath (copy, result, ":", 0, what, where);

  /* Free the copied RPATH string.  `fillin_rpath' make own copies if
     necessary.  */
  free (copy);

  sps->dirs = result;
  /* The caller will change this value if we haven't used a real malloc.  */
  sps->malloced = 1;
}


void
internal_function
_dl_init_paths (const char *llp)
{
  size_t idx;
  const char *strp;
  struct r_search_path_elem *pelem, **aelem;
  size_t round_size;
#ifdef SHARED
  struct link_map *l;
#endif

  /* Fill in the information about the application's RPATH and the
     directories addressed by the LD_LIBRARY_PATH environment variable.  */

  /* Get the capabilities.  */
  capstr = _dl_important_hwcaps (_dl_platform, _dl_platformlen,
				 &ncapstr, &max_capstrlen);

  /* First set up the rest of the default search directory entries.  */
  aelem = rtld_search_dirs.dirs = (struct r_search_path_elem **)
    malloc ((nsystem_dirs_len + 1) * sizeof (struct r_search_path_elem *));
  if (rtld_search_dirs.dirs == NULL)
    _dl_signal_error (ENOMEM, NULL, N_("cannot create search path array"));

  round_size = ((2 * sizeof (struct r_search_path_elem) - 1
		 + ncapstr * sizeof (enum r_dir_status))
		/ sizeof (struct r_search_path_elem));

  rtld_search_dirs.dirs[0] = (struct r_search_path_elem *)
    malloc ((sizeof (system_dirs) / sizeof (system_dirs[0]))
	    * round_size * sizeof (struct r_search_path_elem));
  if (rtld_search_dirs.dirs[0] == NULL)
    _dl_signal_error (ENOMEM, NULL, N_("cannot create cache for search path"));

  rtld_search_dirs.malloced = 0;
  pelem = _dl_all_dirs = rtld_search_dirs.dirs[0];
  strp = system_dirs;
  idx = 0;

  do
    {
      size_t cnt;

      *aelem++ = pelem;

      pelem->what = "system search path";
      pelem->where = NULL;

      pelem->dirname = strp;
      pelem->dirnamelen = system_dirs_len[idx];
      strp += system_dirs_len[idx] + 1;

      /* System paths must be absolute.  */
      assert (pelem->dirname[0] == '/');
      for (cnt = 0; cnt < ncapstr; ++cnt)
	pelem->status[cnt] = unknown;

      pelem->next = (++idx == nsystem_dirs_len ? NULL : (pelem + round_size));

      pelem += round_size;
    }
  while (idx < nsystem_dirs_len);

  max_dirnamelen = SYSTEM_DIRS_MAX_LEN;
  *aelem = NULL;

#ifdef SHARED
  /* This points to the map of the main object.  */
  l = _dl_loaded;
  if (l != NULL)
    {
      assert (l->l_type != lt_loaded);

      if (l->l_info[DT_RUNPATH])
	{
	  /* Allocate room for the search path and fill in information
	     from RUNPATH.  */
	  decompose_rpath (&l->l_runpath_dirs,
			   (const void *) (D_PTR (l, l_info[DT_STRTAB])
					   + l->l_info[DT_RUNPATH]->d_un.d_val),
			   l, "RUNPATH");

	  /* The RPATH is ignored.  */
	  l->l_rpath_dirs.dirs = (void *) -1;
	}
      else
	{
	  l->l_runpath_dirs.dirs = (void *) -1;

	  if (l->l_info[DT_RPATH])
	    {
	      /* Allocate room for the search path and fill in information
		 from RPATH.  */
	      decompose_rpath (&l->l_rpath_dirs,
			       (const void *) (D_PTR (l, l_info[DT_STRTAB])
					       + l->l_info[DT_RPATH]->d_un.d_val),
			       l, "RPATH");
	      l->l_rpath_dirs.malloced = 0;
	    }
	  else
	    l->l_rpath_dirs.dirs = (void *) -1;
	}
    }
#endif	/* SHARED */

  if (llp != NULL && *llp != '\0')
    {
      size_t nllp;
      const char *cp = llp;

      /* Decompose the LD_LIBRARY_PATH contents.  First determine how many
	 elements it has.  */
      nllp = 1;
      while (*cp)
	{
	  if (*cp == ':' || *cp == ';')
	    ++nllp;
	  ++cp;
	}

      env_path_list.dirs = (struct r_search_path_elem **)
	malloc ((nllp + 1) * sizeof (struct r_search_path_elem *));
      if (env_path_list.dirs == NULL)
	_dl_signal_error (ENOMEM, NULL,
			  N_("cannot create cache for search path"));

      (void) fillin_rpath (strdupa (llp), env_path_list.dirs, ":;",
			   __libc_enable_secure, "LD_LIBRARY_PATH", NULL);

      if (env_path_list.dirs[0] == NULL)
	{
	  free (env_path_list.dirs);
	  env_path_list.dirs = (void *) -1;
	}

      env_path_list.malloced = 0;
    }
  else
    env_path_list.dirs = (void *) -1;

  /* Remember the last search directory added at startup.  */
  _dl_init_all_dirs = _dl_all_dirs;
}


/* Think twice before changing anything in this function.  It is placed
   here and prepared using the `alloca' magic to prevent it from being
   inlined.  The function is only called in case of an error.  But then
   performance does not count.  The function used to be "inlinable" and
   the compiled did so all the time.  This increased the code size for
   absolutely no good reason.  */
#define LOSE(code, s) lose (code, fd, name, realname, l, s)
static void
__attribute__ ((noreturn))
lose (int code, int fd, const char *name, char *realname, struct link_map *l,
      const char *msg)
{
  /* The use of `alloca' here looks ridiculous but it helps.  The goal
     is to avoid the function from being inlined.  There is no official
     way to do this so we use this trick.  gcc never inlines functions
     which use `alloca'.  */
  int *a = alloca (sizeof (int));
  a[0] = fd;
  (void) __close (a[0]);
  if (l != NULL)
    {
      /* Remove the stillborn object from the list and free it.  */
      if (l->l_prev)
	l->l_prev->l_next = l->l_next;
      if (l->l_next)
	l->l_next->l_prev = l->l_prev;
      --_dl_nloaded;
      free (l);
    }
  free (realname);
  _dl_signal_error (code, name, msg);
}


/* Map in the shared object NAME, actually located in REALNAME, and already
   opened on FD.  */

#ifndef EXTERNAL_MAP_FROM_FD
static
#endif
struct link_map *
_dl_map_object_from_fd (const char *name, int fd, struct filebuf *fbp,
			char *realname, struct link_map *loader, int l_type,
			int mode)
{
  struct link_map *l = NULL;

  inline caddr_t map_segment (ElfW(Addr) mapstart, size_t len,
			      int prot, int fixed, off_t offset)
    {
      caddr_t mapat = __mmap ((caddr_t) mapstart, len, prot,
			      fixed|MAP_COPY|MAP_FILE,
			      fd, offset);
      if (mapat == MAP_FAILED)
	LOSE (errno, N_("failed to map segment from shared object"));
      return mapat;
    }

  const ElfW(Ehdr) *header;
  const ElfW(Phdr) *phdr;
  const ElfW(Phdr) *ph;
  size_t maplength;
  int type;
  struct stat64 st;

  /* Get file information.  */
  if (__fxstat64 (_STAT_VER, fd, &st) < 0)
    LOSE (errno, N_("cannot stat shared object"));

  /* Look again to see if the real name matched another already loaded.  */
  for (l = _dl_loaded; l; l = l->l_next)
    if (l->l_ino == st.st_ino && l->l_dev == st.st_dev)
      {
	/* The object is already loaded.
	   Just bump its reference count and return it.  */
	__close (fd);

	/* If the name is not in the list of names for this object add
	   it.  */
	free (realname);
	add_name_to_object (l, name);

	return l;
      }

  if (mode & RTLD_NOLOAD)
    /* We are not supposed to load the object unless it is already
       loaded.  So return now.  */
    return NULL;

  /* Print debugging message.  */
  if (__builtin_expect (_dl_debug_files, 0))
    _dl_debug_message (1, "file=", name, ";  generating link map\n", NULL);

  /* This is the ELF header.  We read it in `open_verify'.  */
  header = (void *) fbp->buf;

#ifndef MAP_ANON
# define MAP_ANON 0
  if (_dl_zerofd == -1)
    {
      _dl_zerofd = _dl_sysdep_open_zero_fill ();
      if (_dl_zerofd == -1)
	{
	  __close (fd);
	  _dl_signal_error (errno, NULL, N_("cannot open zero fill device"));
	}
    }
#endif

  /* Enter the new object in the list of loaded objects.  */
  l = _dl_new_object (realname, name, l_type, loader);
  if (__builtin_expect (! l, 0))
    LOSE (ENOMEM, N_("cannot create shared object descriptor"));

  /* Extract the remaining details we need from the ELF header
     and then read in the program header table.  */
  l->l_entry = header->e_entry;
  type = header->e_type;
  l->l_phnum = header->e_phnum;

  maplength = header->e_phnum * sizeof (ElfW(Phdr));
  if (header->e_phoff + maplength <= fbp->len)
    phdr = (void *) (fbp->buf + header->e_phoff);
  else
    {
      phdr = alloca (maplength);
      __lseek (fd, SEEK_SET, header->e_phoff);
      if (__libc_read (fd, (void *) phdr, maplength) != maplength)
        LOSE (errno, N_("cannot read file data"));
    }

  {
    /* Scan the program header table, collecting its load commands.  */
    struct loadcmd
      {
	ElfW(Addr) mapstart, mapend, dataend, allocend;
	off_t mapoff;
	int prot;
      } loadcmds[l->l_phnum], *c;
    size_t nloadcmds = 0;

    /* The struct is initialized to zero so this is not necessary:
    l->l_ld = 0;
    l->l_phdr = 0;
    l->l_addr = 0; */
    for (ph = phdr; ph < &phdr[l->l_phnum]; ++ph)
      switch (ph->p_type)
	{
	  /* These entries tell us where to find things once the file's
	     segments are mapped in.  We record the addresses it says
	     verbatim, and later correct for the run-time load address.  */
	case PT_DYNAMIC:
	  l->l_ld = (void *) ph->p_vaddr;
	  l->l_ldnum = ph->p_memsz / sizeof (ElfW(Dyn));
	  break;
	case PT_PHDR:
	  l->l_phdr = (void *) ph->p_vaddr;
	  break;

	case PT_LOAD:
	  /* A load command tells us to map in part of the file.
	     We record the load commands and process them all later.  */
	  if (ph->p_align % _dl_pagesize != 0)
	    LOSE (0, N_("ELF load command alignment not page-aligned"));
	  if ((ph->p_vaddr - ph->p_offset) % ph->p_align)
	    LOSE (0,
		  N_("ELF load command address/offset not properly aligned"));
	  {
	    struct loadcmd *c = &loadcmds[nloadcmds++];
	    c->mapstart = ph->p_vaddr & ~(ph->p_align - 1);
	    c->mapend = ((ph->p_vaddr + ph->p_filesz + _dl_pagesize - 1)
			 & ~(_dl_pagesize - 1));
	    c->dataend = ph->p_vaddr + ph->p_filesz;
	    c->allocend = ph->p_vaddr + ph->p_memsz;
	    c->mapoff = ph->p_offset & ~(ph->p_align - 1);

	    /* Optimize a common case.  */
	    if ((PF_R | PF_W | PF_X) == 7
		&& (PROT_READ | PROT_WRITE | PROT_EXEC) == 7)
	      c->prot = _dl_pf_to_prot[ph->p_flags & (PF_R | PF_W | PF_X)];
	    else
	      {
		c->prot = 0;
		if (ph->p_flags & PF_R)
		  c->prot |= PROT_READ;
		if (ph->p_flags & PF_W)
		  c->prot |= PROT_WRITE;
		if (ph->p_flags & PF_X)
		  c->prot |= PROT_EXEC;
	      }
	    break;
	  }
	}

    /* Now process the load commands and map segments into memory.  */
    c = loadcmds;

    /* Length of the sections to be loaded.  */
    maplength = loadcmds[nloadcmds - 1].allocend - c->mapstart;

    if (__builtin_expect (type, ET_DYN) == ET_DYN)
      {
	/* This is a position-independent shared object.  We can let the
	   kernel map it anywhere it likes, but we must have space for all
	   the segments in their specified positions relative to the first.
	   So we map the first segment without MAP_FIXED, but with its
	   extent increased to cover all the segments.  Then we remove
	   access from excess portion, and there is known sufficient space
	   there to remap from the later segments.

	   As a refinement, sometimes we have an address that we would
	   prefer to map such objects at; but this is only a preference,
	   the OS can do whatever it likes. */
	ElfW(Addr) mappref;
	mappref = (ELF_PREFERRED_ADDRESS (loader, maplength, c->mapstart)
		   - MAP_BASE_ADDR (l));

	/* Remember which part of the address space this object uses.  */
	l->l_map_start = (ElfW(Addr)) map_segment (mappref, maplength, c->prot,
						   0, c->mapoff);
	l->l_map_end = l->l_map_start + maplength;
	l->l_addr = l->l_map_start - c->mapstart;

	/* Change protection on the excess portion to disallow all access;
	   the portions we do not remap later will be inaccessible as if
	   unallocated.  Then jump into the normal segment-mapping loop to
	   handle the portion of the segment past the end of the file
	   mapping.  */
	__mprotect ((caddr_t) (l->l_addr + c->mapend),
		    loadcmds[nloadcmds - 1].allocend - c->mapend,
		    0);

	goto postmap;
      }
    else
      {
	/* This object is loaded at a fixed address.  This must never
           happen for objects loaded with dlopen().  */
	if (mode & __RTLD_DLOPEN)
	  {
	    LOSE (0, N_("cannot dynamically load executable"));
	  }

	/* Notify ELF_PREFERRED_ADDRESS that we have to load this one
	   fixed.  */
	ELF_FIXED_ADDRESS (loader, c->mapstart);
      }

    /* Remember which part of the address space this object uses.  */
    l->l_map_start = c->mapstart + l->l_addr;
    l->l_map_end = l->l_map_start + maplength;

    while (c < &loadcmds[nloadcmds])
      {
	if (c->mapend > c->mapstart)
	  /* Map the segment contents from the file.  */
	  map_segment (l->l_addr + c->mapstart, c->mapend - c->mapstart,
		       c->prot, MAP_FIXED, c->mapoff);

      postmap:
	if (l->l_phdr == 0
	    && c->mapoff <= header->e_phoff
	    && (c->mapend - c->mapstart + c->mapoff
		>= header->e_phoff + header->e_phnum * sizeof (ElfW(Phdr))))
	  /* Found the program header in this segment.  */
	  l->l_phdr = (void *) (c->mapstart + header->e_phoff - c->mapoff);

	if (c->allocend > c->dataend)
	  {
	    /* Extra zero pages should appear at the end of this segment,
	       after the data mapped from the file.   */
	    ElfW(Addr) zero, zeroend, zeropage;

	    zero = l->l_addr + c->dataend;
	    zeroend = l->l_addr + c->allocend;
	    zeropage = (zero + _dl_pagesize - 1) & ~(_dl_pagesize - 1);

	    if (zeroend < zeropage)
	      /* All the extra data is in the last page of the segment.
		 We can just zero it.  */
	      zeropage = zeroend;

	    if (zeropage > zero)
	      {
		/* Zero the final part of the last page of the segment.  */
		if ((c->prot & PROT_WRITE) == 0)
		  {
		    /* Dag nab it.  */
		    if (__mprotect ((caddr_t) (zero & ~(_dl_pagesize - 1)),
				    _dl_pagesize, c->prot|PROT_WRITE) < 0)
		      LOSE (errno, N_("cannot change memory protections"));
		  }
		memset ((void *) zero, 0, zeropage - zero);
		if ((c->prot & PROT_WRITE) == 0)
		  __mprotect ((caddr_t) (zero & ~(_dl_pagesize - 1)),
			      _dl_pagesize, c->prot);
	      }

	    if (zeroend > zeropage)
	      {
		/* Map the remaining zero pages in from the zero fill FD.  */
		caddr_t mapat;
		mapat = __mmap ((caddr_t) zeropage, zeroend - zeropage,
				c->prot, MAP_ANON|MAP_PRIVATE|MAP_FIXED,
				ANONFD, 0);
		if (mapat == MAP_FAILED)
		  LOSE (errno, N_("cannot map zero-fill pages"));
	      }
	  }

	++c;
      }

    if (l->l_phdr == NULL)
      {
	/* The program header is not contained in any of the segmenst.
	   We have to allocate memory ourself and copy it over from
	   out temporary place.  */
	ElfW(Phdr) *newp = (ElfW(Phdr) *) malloc (header->e_phnum
						  * sizeof (ElfW(Phdr)));
	if (newp == NULL)
	  LOSE (ENOMEM, N_("cannot allocate memory for program header"));

	l->l_phdr = memcpy (newp, phdr,
			    (header->e_phnum * sizeof (ElfW(Phdr))));
	l->l_phdr_allocated = 1;
      }
    else
      /* Adjust the PT_PHDR value by the runtime load address.  */
      (ElfW(Addr)) l->l_phdr += l->l_addr;
  }

  /* We are done mapping in the file.  We no longer need the descriptor.  */
  __close (fd);

  if (l->l_type == lt_library && type == ET_EXEC)
    l->l_type = lt_executable;

  if (l->l_ld == 0)
    {
      if (type == ET_DYN)
	LOSE (0, N_("object file has no dynamic section"));
    }
  else
    (ElfW(Addr)) l->l_ld += l->l_addr;

  l->l_entry += l->l_addr;

  if (__builtin_expect (_dl_debug_files, 0))
    {
      const size_t nibbles = sizeof (void *) * 2;
      char buf1[nibbles + 1];
      char buf2[nibbles + 1];
      char buf3[nibbles + 1];

      buf1[nibbles] = '\0';
      buf2[nibbles] = '\0';
      buf3[nibbles] = '\0';

      memset (buf1, '0', nibbles);
      memset (buf2, '0', nibbles);
      memset (buf3, '0', nibbles);
      _itoa_word ((unsigned long int) l->l_ld, &buf1[nibbles], 16, 0);
      _itoa_word ((unsigned long int) l->l_addr, &buf2[nibbles], 16, 0);
      _itoa_word (maplength, &buf3[nibbles], 16, 0);

      _dl_debug_message (1, "  dynamic: 0x", buf1, "  base: 0x", buf2,
			 "   size: 0x", buf3, "\n", NULL);
      memset (buf1, '0', nibbles);
      memset (buf2, '0', nibbles);
      memset (buf3, ' ', nibbles);
      _itoa_word ((unsigned long int) l->l_entry, &buf1[nibbles], 16, 0);
      _itoa_word ((unsigned long int) l->l_phdr, &buf2[nibbles], 16, 0);
      _itoa_word (l->l_phnum, &buf3[nibbles], 10, 0);
      _dl_debug_message (1, "    entry: 0x", buf1, "  phdr: 0x", buf2,
			 "  phnum:   ", buf3, "\n\n", NULL);
    }

  elf_get_dynamic_info (l);

  /* Make sure we are dlopen()ing an object which has the DF_1_NOOPEN
     flag set.  */
  if (__builtin_expect (l->l_flags_1 & DF_1_NOOPEN, 0)
      && (mode & __RTLD_DLOPEN))
    {
      /* Remove from the module list.  */
      assert (l->l_next == NULL);
#ifndef SHARED
      if (l->l_prev == NULL)
	/* No other module loaded.  */
	_dl_loaded = NULL;
      else
#endif
	l->l_prev->l_next = NULL;
      --_dl_nloaded;

      /* We are not supposed to load this object.  Free all resources.  */
      __munmap ((void *) l->l_map_start, l->l_map_end - l->l_map_start);

      free (l->l_libname);

      if (l->l_phdr_allocated)
	free ((void *) l->l_phdr);

      free (l);

      _dl_signal_error (0, name, N_("shared object cannot be dlopen()ed"));
    }

  if (l->l_info[DT_HASH])
    _dl_setup_hash (l);

  /* If this object has DT_SYMBOLIC set modify now its scope.  We don't
     have to do this for the main map.  */
  if (__builtin_expect (l->l_info[DT_SYMBOLIC] != NULL, 0)
	   && &l->l_searchlist != l->l_scope[0])
    {
      /* Create an appropriate searchlist.  It contains only this map.

	 XXX This is the definition of DT_SYMBOLIC in SysVr4.  The old
	 GNU ld.so implementation had a different interpretation which
	 is more reasonable.  We are prepared to add this possibility
	 back as part of a GNU extension of the ELF format.  */
      l->l_symbolic_searchlist.r_list =
	(struct link_map **) malloc (sizeof (struct link_map *));

      if (l->l_symbolic_searchlist.r_list == NULL)
	LOSE (ENOMEM, N_("cannot create searchlist"));

      l->l_symbolic_searchlist.r_list[0] = l;
      l->l_symbolic_searchlist.r_nlist = 1;
      l->l_symbolic_searchlist.r_duplist = l->l_symbolic_searchlist.r_list;
      l->l_symbolic_searchlist.r_nduplist = 1;

      /* Now move the existing entries one back.  */
      memmove (&l->l_scope[1], &l->l_scope[0],
	       sizeof (l->l_scope) - sizeof (l->l_scope[0]));

      /* Now add the new entry.  */
      l->l_scope[0] = &l->l_symbolic_searchlist;
    }

  /* Finally the file information.  */
  l->l_dev = st.st_dev;
  l->l_ino = st.st_ino;

  return l;
}

/* Print search path.  */
static void
print_search_path (struct r_search_path_elem **list,
                   const char *what, const char *name)
{
  char buf[max_dirnamelen + max_capstrlen];
  int first = 1;

  _dl_debug_message (1, " search path=", NULL);

  while (*list != NULL && (*list)->what == what) /* Yes, ==.  */
    {
      char *endp = __mempcpy (buf, (*list)->dirname, (*list)->dirnamelen);
      size_t cnt;

      for (cnt = 0; cnt < ncapstr; ++cnt)
	if ((*list)->status[cnt] != nonexisting)
	  {
	    char *cp = __mempcpy (endp, capstr[cnt].str, capstr[cnt].len);
	    if (cp == buf || (cp == buf + 1 && buf[0] == '/'))
	      cp[0] = '\0';
	    else
	      cp[-1] = '\0';
	    _dl_debug_message (0, first ? "" : ":", buf, NULL);
	    first = 0;
	  }

      ++list;
    }

  if (name != NULL)
    _dl_debug_message (0, "\t\t(", what, " from file ",
			name[0] ? name : _dl_argv[0], ")\n", NULL);
  else
    _dl_debug_message (0, "\t\t(", what, ")\n", NULL);
}

/* Open a file and verify it is an ELF file for this architecture.  We
   ignore only ELF files for other architectures.  Non-ELF files and
   ELF files with different header information cause fatal errors since
   this could mean there is something wrong in the installation and the
   user might want to know about this.  */
static int
open_verify (const char *name, struct filebuf *fbp)
{
  /* This is the expected ELF header.  */
#define ELF32_CLASS ELFCLASS32
#define ELF64_CLASS ELFCLASS64
#ifndef VALID_ELF_HEADER
# define VALID_ELF_HEADER(hdr,exp,size)	(memcmp (hdr, exp, size) == 0)
# define VALID_ELF_OSABI(osabi)		(osabi == ELFOSABI_SYSV)
# define VALID_ELF_ABIVERSION(ver)	(ver == 0)
#endif
  static const unsigned char expected[EI_PAD] =
  {
    [EI_MAG0] = ELFMAG0,
    [EI_MAG1] = ELFMAG1,
    [EI_MAG2] = ELFMAG2,
    [EI_MAG3] = ELFMAG3,
    [EI_CLASS] = ELFW(CLASS),
    [EI_DATA] = byteorder,
    [EI_VERSION] = EV_CURRENT,
    [EI_OSABI] = ELFOSABI_SYSV,
    [EI_ABIVERSION] = 0
  };
  int fd;

  /* Open the file.  We always open files read-only.  */
  fd = __open (name, O_RDONLY);
  if (fd != -1)
    {
      ElfW(Ehdr) *ehdr;

      /* We successfully openened the file.  Now verify it is a file
	 we can use.  */
      __set_errno (0);
      fbp->len = __libc_read (fd, fbp->buf, sizeof (fbp->buf));

      /* This is where the ELF header is loaded.  */
      assert (sizeof (fbp->buf) > sizeof (ElfW(Ehdr)));
      ehdr = (ElfW(Ehdr) *) fbp->buf;

      /* Now run the tests.  */
      if (__builtin_expect (fbp->len < (ssize_t) sizeof (ElfW(Ehdr)), 0))
	lose (errno, fd, name, NULL, NULL,
	      errno == 0 ? N_("file too short") : N_("cannot read file data"));

      /* See whether the ELF header is what we expect.  */
      if (__builtin_expect (! VALID_ELF_HEADER (ehdr->e_ident, expected,
						EI_PAD), 0))
	{
	  /* Something is wrong.  */
	  if (*(Elf32_Word *) &ehdr->e_ident !=
#if BYTE_ORDER == LITTLE_ENDIAN
	      ((ELFMAG0 << (EI_MAG0 * 8)) |
	       (ELFMAG1 << (EI_MAG1 * 8)) |
	       (ELFMAG2 << (EI_MAG2 * 8)) |
	       (ELFMAG3 << (EI_MAG3 * 8)))
#else
	      ((ELFMAG0 << (EI_MAG3 * 8)) |
	       (ELFMAG1 << (EI_MAG2 * 8)) |
	       (ELFMAG2 << (EI_MAG1 * 8)) |
	       (ELFMAG3 << (EI_MAG0 * 8)))
#endif
	      )
	    lose (0, fd, name, NULL, NULL, N_("invalid ELF header"));

	  if (ehdr->e_ident[EI_CLASS] != ELFW(CLASS))
	    /* This is not a fatal error.  On architectures where
	       32-bit and 64-bit binaries can be run this might
	       happen.  */
	    goto close_and_out;

	  if (ehdr->e_ident[EI_DATA] != byteorder)
	    {
	      if (BYTE_ORDER == BIG_ENDIAN)
		lose (0, fd, name, NULL, NULL,
		      "ELF file data encoding not big-endian");
	      else
		lose (0, fd, name, NULL, NULL,
		      "ELF file data encoding not little-endian");
	    }
	  if (ehdr->e_ident[EI_VERSION] != EV_CURRENT)
	    lose (0, fd, name, NULL, NULL,
		  N_("ELF file version ident does not match current one"));
	  /* XXX We should be able so set system specific versions which are
	     allowed here.  */
	  if (!VALID_ELF_OSABI (ehdr->e_ident[EI_OSABI]))
	    lose (0, fd, name, NULL, NULL, N_("ELF file OS ABI invalid"));
	  if (!VALID_ELF_ABIVERSION (ehdr->e_ident[EI_ABIVERSION]))
	    lose (0, fd, name, NULL, NULL,
		  N_("ELF file ABI version invalid"));
	  lose (0, fd, name, NULL, NULL, N_("internal error"));
	}

      if (__builtin_expect (ehdr->e_version, EV_CURRENT) != EV_CURRENT)
	lose (0, fd, name, NULL, NULL,
	      N_("ELF file version does not match current one"));
      if (! __builtin_expect (elf_machine_matches_host (ehdr), 1))
	{
	close_and_out:
	  __close (fd);
	  __set_errno (ENOENT);
	  fd = -1;
	}
      else if (__builtin_expect (ehdr->e_phentsize, sizeof (ElfW(Phdr)))
	       != sizeof (ElfW(Phdr)))
	lose (0, fd, name, NULL, NULL,
	      N_("ELF file's phentsize not the expected size"));
      else if (__builtin_expect (ehdr->e_type, ET_DYN) != ET_DYN
	       && __builtin_expect (ehdr->e_type, ET_EXEC) != ET_EXEC)
	lose (0, fd, name, NULL, NULL,
	      N_("only ET_DYN and ET_EXEC can be loaded"));
    }

  return fd;
}

/* Try to open NAME in one of the directories in *DIRSP.
   Return the fd, or -1.  If successful, fill in *REALNAME
   with the malloc'd full directory name.  If it turns out
   that none of the directories in *DIRSP exists, *DIRSP is
   replaced with (void *) -1, and the old value is free()d
   if MAY_FREE_DIRS is true.  */

static int
open_path (const char *name, size_t namelen, int preloaded,
	   struct r_search_path_struct *sps, char **realname,
	   struct filebuf *fbp)
{
  struct r_search_path_elem **dirs = sps->dirs;
  char *buf;
  int fd = -1;
  const char *current_what = NULL;
  int any = 0;

  buf = alloca (max_dirnamelen + max_capstrlen + namelen);
  do
    {
      struct r_search_path_elem *this_dir = *dirs;
      size_t buflen = 0;
      size_t cnt;
      char *edp;
      int here_any = 0;

      /* If we are debugging the search for libraries print the path
	 now if it hasn't happened now.  */
      if (__builtin_expect (_dl_debug_libs, 0)
	  && current_what != this_dir->what)
	{
	  current_what = this_dir->what;
	  print_search_path (dirs, current_what, this_dir->where);
	}

      edp = (char *) __mempcpy (buf, this_dir->dirname, this_dir->dirnamelen);
      for (cnt = 0; fd == -1 && cnt < ncapstr; ++cnt)
	{
	  /* Skip this directory if we know it does not exist.  */
	  if (this_dir->status[cnt] == nonexisting)
	    continue;

	  buflen =
	    ((char *) __mempcpy (__mempcpy (edp,
					    capstr[cnt].str, capstr[cnt].len),
				 name, namelen)
	     - buf);

	  /* Print name we try if this is wanted.  */
	  if (__builtin_expect (_dl_debug_libs, 0))
	    _dl_debug_message (1, "  trying file=", buf, "\n", NULL);

	  fd = open_verify (buf, fbp);
	  if (this_dir->status[cnt] == unknown)
	    {
	      if (fd != -1)
		this_dir->status[cnt] = existing;
	      else
		{
		  /* We failed to open machine dependent library.  Let's
		     test whether there is any directory at all.  */
		  struct stat64 st;

		  buf[buflen - namelen - 1] = '\0';

		  if (__xstat64 (_STAT_VER, buf, &st) != 0
		      || ! S_ISDIR (st.st_mode))
		    /* The directory does not exist or it is no directory.  */
		    this_dir->status[cnt] = nonexisting;
		  else
		    this_dir->status[cnt] = existing;
		}
	    }

	  /* Remember whether we found any existing directory.  */
	  here_any |= this_dir->status[cnt] == existing;

	  if (fd != -1 && preloaded && __libc_enable_secure)
	    {
	      /* This is an extra security effort to make sure nobody can
		 preload broken shared objects which are in the trusted
		 directories and so exploit the bugs.  */
	      struct stat64 st;

	      if (__fxstat64 (_STAT_VER, fd, &st) != 0
		  || (st.st_mode & S_ISUID) == 0)
		{
		  /* The shared object cannot be tested for being SUID
		     or this bit is not set.  In this case we must not
		     use this object.  */
		  __close (fd);
		  fd = -1;
		  /* We simply ignore the file, signal this by setting
		     the error value which would have been set by `open'.  */
		  errno = ENOENT;
		}
	    }
	}

      if (fd != -1)
	{
	  *realname = malloc (buflen);
	  if (*realname != NULL)
	    {
	      memcpy (*realname, buf, buflen);
	      return fd;
	    }
	  else
	    {
	      /* No memory for the name, we certainly won't be able
		 to load and link it.  */
	      __close (fd);
	      return -1;
	    }
	}
      if (here_any && errno != ENOENT && errno != EACCES)
	/* The file exists and is readable, but something went wrong.  */
	return -1;

      /* Remember whether we found anything.  */
      any |= here_any;
    }
  while (*++dirs != NULL);

  /* Remove the whole path if none of the directories exists.  */
  if (! any)
    {
      /* Paths which were allocated using the minimal malloc() in ld.so
	 must not be freed using the general free() in libc.  */
      if (sps->malloced)
	free (sps->dirs);
      sps->dirs = (void *) -1;
    }

  return -1;
}

/* Map in the shared object file NAME.  */

struct link_map *
internal_function
_dl_map_object (struct link_map *loader, const char *name, int preloaded,
		int type, int trace_mode, int mode)
{
  int fd;
  char *realname;
  char *name_copy;
  struct link_map *l;
  struct filebuf fb;

  /* Look for this name among those already loaded.  */
  for (l = _dl_loaded; l; l = l->l_next)
    {
      /* If the requested name matches the soname of a loaded object,
	 use that object.  Elide this check for names that have not
	 yet been opened.  */
      /* XXX Is this test still correct after the reference counter
	 handling rewrite?  */
      if (l->l_opencount == 0)
	continue;
      if (!_dl_name_match_p (name, l))
	{
	  const char *soname;

	  if (__builtin_expect (l->l_soname_added, 1)
	      || l->l_info[DT_SONAME] == NULL)
	    continue;

	  soname = ((const char *) D_PTR (l, l_info[DT_STRTAB])
		    + l->l_info[DT_SONAME]->d_un.d_val);
	  if (strcmp (name, soname) != 0)
	    continue;

	  /* We have a match on a new name -- cache it.  */
	  add_name_to_object (l, soname);
	  l->l_soname_added = 1;
	}

      /* We have a match.  */
      return l;
    }

  /* Display information if we are debugging.  */
  if (__builtin_expect (_dl_debug_files, 0) && loader != NULL)
    _dl_debug_message (1, "\nfile=", name, ";  needed by ",
		       loader->l_name[0] ? loader->l_name : _dl_argv[0],
		       "\n", NULL);

  if (strchr (name, '/') == NULL)
    {
      /* Search for NAME in several places.  */

      size_t namelen = strlen (name) + 1;

      if (__builtin_expect (_dl_debug_libs, 0))
	_dl_debug_message (1, "find library=", name, "; searching\n", NULL);

      fd = -1;

      /* When the object has the RUNPATH information we don't use any
         RPATHs.  */
      if (loader == NULL || loader->l_info[DT_RUNPATH] == NULL)
	{
	  /* First try the DT_RPATH of the dependent object that caused NAME
	     to be loaded.  Then that object's dependent, and on up.  */
	  for (l = loader; fd == -1 && l; l = l->l_loader)
	    {
	      if (l->l_rpath_dirs.dirs == NULL)
		{
		  if (l->l_info[DT_RPATH] == NULL)
		    /* There is no path.  */
		    l->l_rpath_dirs.dirs = (void *) -1;
		  else
		    {
		      /* Make sure the cache information is available.  */
		      size_t ptrval = (D_PTR (l, l_info[DT_STRTAB])
				       + l->l_info[DT_RPATH]->d_un.d_val);
		      decompose_rpath (&l->l_rpath_dirs,
				       (const char *) ptrval, l, "RPATH");

		      if (l->l_rpath_dirs.dirs != (void *) -1)
			fd = open_path (name, namelen, preloaded,
					&l->l_rpath_dirs, &realname, &fb);
		    }
		}
	      else if (l->l_rpath_dirs.dirs != (void *) -1)
		fd = open_path (name, namelen, preloaded, &l->l_rpath_dirs,
				&realname, &fb);
	    }

	  /* If dynamically linked, try the DT_RPATH of the executable
             itself.  */
	  l = _dl_loaded;
	  if (fd == -1 && l && l->l_type != lt_loaded && l != loader
	      && l->l_rpath_dirs.dirs != (void *) -1)
	    fd = open_path (name, namelen, preloaded, &l->l_rpath_dirs,
			    &realname, &fb);
	}

      /* Try the LD_LIBRARY_PATH environment variable.  */
      if (fd == -1 && env_path_list.dirs != (void *) -1)
	fd = open_path (name, namelen, preloaded, &env_path_list,
			&realname, &fb);

      /* Look at the RUNPATH informaiton for this binary.  */
      if (loader != NULL && loader->l_runpath_dirs.dirs != (void *) -1)
	{
	  if (loader->l_runpath_dirs.dirs == NULL)
	    {
	      if (loader->l_info[DT_RUNPATH] == NULL)
		/* No RUNPATH.  */
		loader->l_runpath_dirs.dirs = (void *) -1;
	      else
		{
		  /* Make sure the cache information is available.  */
		  size_t ptrval = (D_PTR (loader, l_info[DT_STRTAB])
				   + loader->l_info[DT_RUNPATH]->d_un.d_val);
		  decompose_rpath (&loader->l_runpath_dirs,
				   (const char *) ptrval, loader, "RUNPATH");

		  if (loader->l_runpath_dirs.dirs != (void *) -1)
		    fd = open_path (name, namelen, preloaded,
				    &loader->l_runpath_dirs, &realname, &fb);
		}
	    }
	  else if (loader->l_runpath_dirs.dirs != (void *) -1)
	    fd = open_path (name, namelen, preloaded,
			    &loader->l_runpath_dirs, &realname, &fb);
	}

      if (fd == -1)
	{
	  /* Check the list of libraries in the file /etc/ld.so.cache,
	     for compatibility with Linux's ldconfig program.  */
	  const char *cached = _dl_load_cache_lookup (name);

#ifdef SHARED
	  l = loader ?: _dl_loaded;
#else
	  l = loader;
#endif

	  if (cached)
	    {
	      /* If the loader has the DF_1_NODEFLIB flag set we must not
		 use a cache entry from any of these directories.  */
	      if (l && __builtin_expect (l->l_flags_1 & DF_1_NODEFLIB, 0))
		{
		  const char *dirp = system_dirs;
		  int cnt = 0;

		  do
		    {
		      if (memcmp (cached, dirp, system_dirs_len[cnt]) == 0)
			{
			  /* The prefix matches.  Don't use the entry.  */
			  cached = NULL;
			  break;
			}

		      dirp += system_dirs_len[cnt] + 1;
		      ++cnt;
		    }
		  while (cnt < nsystem_dirs_len);
		}

	      if (cached)
		{
		  fd = open_verify (cached, &fb);
		  if (fd != -1)
		    {
		      realname = local_strdup (cached);
		      if (realname == NULL)
			{
			  __close (fd);
			  fd = -1;
			}
		    }
		}
	    }
	}

      /* Finally, try the default path.  */
      if (fd == -1
	  && (l == NULL ||
	      __builtin_expect (!(l->l_flags_1 & DF_1_NODEFLIB), 1))
	  && rtld_search_dirs.dirs != (void *) -1)
	fd = open_path (name, namelen, preloaded, &rtld_search_dirs,
			&realname, &fb);

      /* Add another newline when we a tracing the library loading.  */
      if (__builtin_expect (_dl_debug_libs, 0))
        _dl_debug_message (1, "\n", NULL);
    }
  else
    {
      /* The path may contain dynamic string tokens.  */
      realname = (loader
		  ? expand_dynamic_string_token (loader, name)
		  : local_strdup (name));
      if (realname == NULL)
	fd = -1;
      else
	{
	  fd = open_verify (realname, &fb);
	  if (fd == -1)
	    free (realname);
	}
    }

  if (fd == -1)
    {
      if (trace_mode)
	{
	  /* We haven't found an appropriate library.  But since we
	     are only interested in the list of libraries this isn't
	     so severe.  Fake an entry with all the information we
	     have.  */
	  static const Elf_Symndx dummy_bucket = STN_UNDEF;

	  /* Enter the new object in the list of loaded objects.  */
	  if ((name_copy = local_strdup (name)) == NULL
	      || (l = _dl_new_object (name_copy, name, type, loader)) == NULL)
	    _dl_signal_error (ENOMEM, name,
			      N_("cannot create shared object descriptor"));
	  /* Signal that this is a faked entry.  */
	  l->l_faked = 1;
	  /* Since the descriptor is initialized with zero we do not
	     have do this here.
	  l->l_reserved = 0; */
	  l->l_buckets = &dummy_bucket;
	  l->l_nbuckets = 1;
	  l->l_relocated = 1;

	  return l;
	}
      else
	_dl_signal_error (errno, name, N_("cannot open shared object file"));
    }

  return _dl_map_object_from_fd (name, fd, &fb, realname, loader, type, mode);
}
