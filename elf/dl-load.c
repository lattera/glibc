/* Map in a shared object's segments from the file.
   Copyright (C) 1995-2016 Free Software Foundation, Inc.
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

#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <libintl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ldsodefs.h>
#include <bits/wordsize.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "dynamic-link.h"
#include <abi-tag.h>
#include <stackinfo.h>
#include <caller.h>
#include <sysdep.h>
#include <stap-probe.h>
#include <libc-internal.h>

#include <dl-dst.h>
#include <dl-load.h>
#include <dl-map-segments.h>
#include <dl-unmap-segments.h>
#include <dl-machine-reject-phdr.h>
#include <dl-sysdep-open.h>


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


int __stack_prot attribute_hidden attribute_relro
#if _STACK_GROWS_DOWN && defined PROT_GROWSDOWN
  = PROT_GROWSDOWN;
#elif _STACK_GROWS_UP && defined PROT_GROWSUP
  = PROT_GROWSUP;
#else
  = 0;
#endif


/* Type for the buffer we put the ELF header and hopefully the program
   header.  This buffer does not really have to be too large.  In most
   cases the program header follows the ELF header directly.  If this
   is not the case all bets are off and we can make the header
   arbitrarily large and still won't get it read.  This means the only
   question is how large are the ELF and program header combined.  The
   ELF header 32-bit files is 52 bytes long and in 64-bit files is 64
   bytes long.  Each program header entry is again 32 and 56 bytes
   long respectively.  I.e., even with a file which has 10 program
   header entries we only have to read 372B/624B respectively.  Add to
   this a bit of margin for program notes and reading 512B and 832B
   for 32-bit and 64-bit files respecitvely is enough.  If this
   heuristic should really fail for some file the code in
   `_dl_map_object_from_fd' knows how to recover.  */
struct filebuf
{
  ssize_t len;
#if __WORDSIZE == 32
# define FILEBUF_SIZE 512
#else
# define FILEBUF_SIZE 832
#endif
  char buf[FILEBUF_SIZE] __attribute__ ((aligned (__alignof (ElfW(Ehdr)))));
};

/* This is the decomposed LD_LIBRARY_PATH search path.  */
static struct r_search_path_struct env_path_list attribute_relro;

/* List of the hardware capabilities we might end up using.  */
static const struct r_strlenpair *capstr attribute_relro;
static size_t ncapstr attribute_relro;
static size_t max_capstrlen attribute_relro;


/* Get the generated information about the trusted directories.  */
#include "trusted-dirs.h"

static const char system_dirs[] = SYSTEM_DIRS;
static const size_t system_dirs_len[] =
{
  SYSTEM_DIRS_LEN
};
#define nsystem_dirs_len \
  (sizeof (system_dirs_len) / sizeof (system_dirs_len[0]))


static bool
is_trusted_path (const char *path, size_t len)
{
  const char *trun = system_dirs;

  for (size_t idx = 0; idx < nsystem_dirs_len; ++idx)
    {
      if (len == system_dirs_len[idx] && memcmp (trun, path, len) == 0)
	/* Found it.  */
	return true;

      trun += system_dirs_len[idx] + 1;
    }

  return false;
}


static bool
is_trusted_path_normalize (const char *path, size_t len)
{
  if (len == 0)
    return false;

  if (*path == ':')
    {
      ++path;
      --len;
    }

  char *npath = (char *) alloca (len + 2);
  char *wnp = npath;
  while (*path != '\0')
    {
      if (path[0] == '/')
	{
	  if (path[1] == '.')
	    {
	      if (path[2] == '.' && (path[3] == '/' || path[3] == '\0'))
		{
		  while (wnp > npath && *--wnp != '/')
		    ;
		  path += 3;
		  continue;
		}
	      else if (path[2] == '/' || path[2] == '\0')
		{
		  path += 2;
		  continue;
		}
	    }

	  if (wnp > npath && wnp[-1] == '/')
	    {
	      ++path;
	      continue;
	    }
	}

      *wnp++ = *path++;
    }

  if (wnp == npath || wnp[-1] != '/')
    *wnp++ = '/';

  const char *trun = system_dirs;

  for (size_t idx = 0; idx < nsystem_dirs_len; ++idx)
    {
      if (wnp - npath >= system_dirs_len[idx]
	  && memcmp (trun, npath, system_dirs_len[idx]) == 0)
	/* Found it.  */
	return true;

      trun += system_dirs_len[idx] + 1;
    }

  return false;
}


static size_t
is_dst (const char *start, const char *name, const char *str,
	int is_path, int secure)
{
  size_t len;
  bool is_curly = false;

  if (name[0] == '{')
    {
      is_curly = true;
      ++name;
    }

  len = 0;
  while (name[len] == str[len] && name[len] != '\0')
    ++len;

  if (is_curly)
    {
      if (name[len] != '}')
	return 0;

      /* Point again at the beginning of the name.  */
      --name;
      /* Skip over closing curly brace and adjust for the --name.  */
      len += 2;
    }
  else if (name[len] != '\0' && name[len] != '/'
	   && (!is_path || name[len] != ':'))
    return 0;

  if (__glibc_unlikely (secure)
      && ((name[len] != '\0' && name[len] != '/'
	   && (!is_path || name[len] != ':'))
	  || (name != start + 1 && (!is_path || name[-2] != ':'))))
    return 0;

  return len;
}


size_t
_dl_dst_count (const char *name, int is_path)
{
  const char *const start = name;
  size_t cnt = 0;

  do
    {
      size_t len;

      /* $ORIGIN is not expanded for SUID/GUID programs (except if it
	 is $ORIGIN alone) and it must always appear first in path.  */
      ++name;
      if ((len = is_dst (start, name, "ORIGIN", is_path,
			 __libc_enable_secure)) != 0
	  || (len = is_dst (start, name, "PLATFORM", is_path, 0)) != 0
	  || (len = is_dst (start, name, "LIB", is_path, 0)) != 0)
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

  /* Now fill the result path.  While copying over the string we keep
     track of the start of the last path element.  When we come across
     a DST we copy over the value or (if the value is not available)
     leave the entire path element out.  */
  char *wp = result;
  char *last_elem = result;
  bool check_for_trusted = false;

  do
    {
      if (__glibc_unlikely (*name == '$'))
	{
	  const char *repl = NULL;
	  size_t len;

	  ++name;
	  if ((len = is_dst (start, name, "ORIGIN", is_path,
			     __libc_enable_secure)) != 0)
	    {
	      repl = l->l_origin;
	      check_for_trusted = (__libc_enable_secure
				   && l->l_type == lt_executable);
	    }
	  else if ((len = is_dst (start, name, "PLATFORM", is_path, 0)) != 0)
	    repl = GLRO(dl_platform);
	  else if ((len = is_dst (start, name, "LIB", is_path, 0)) != 0)
	    repl = DL_DST_LIB;

	  if (repl != NULL && repl != (const char *) -1)
	    {
	      wp = __stpcpy (wp, repl);
	      name += len;
	    }
	  else if (len > 1)
	    {
	      /* We cannot use this path element, the value of the
		 replacement is unknown.  */
	      wp = last_elem;
	      name += len;
	      while (*name != '\0' && (!is_path || *name != ':'))
		++name;
	      /* Also skip following colon if this is the first rpath
		 element, but keep an empty element at the end.  */
	      if (wp == result && is_path && *name == ':' && name[1] != '\0')
		++name;
	    }
	  else
	    /* No DST we recognize.  */
	    *wp++ = '$';
	}
      else
	{
	  *wp++ = *name++;
	  if (is_path && *name == ':')
	    {
	      /* In SUID/SGID programs, after $ORIGIN expansion the
		 normalized path must be rooted in one of the trusted
		 directories.  */
	      if (__glibc_unlikely (check_for_trusted)
		  && !is_trusted_path_normalize (last_elem, wp - last_elem))
		wp = last_elem;
	      else
		last_elem = wp;

	      check_for_trusted = false;
	    }
	}
    }
  while (*name != '\0');

  /* In SUID/SGID programs, after $ORIGIN expansion the normalized
     path must be rooted in one of the trusted directories.  */
  if (__glibc_unlikely (check_for_trusted)
      && !is_trusted_path_normalize (last_elem, wp - last_elem))
    wp = last_elem;

  *wp = '\0';

  return result;
}


/* Return copy of argument with all recognized dynamic string tokens
   ($ORIGIN and $PLATFORM for now) replaced.  On some platforms it
   might not be possible to determine the path from which the object
   belonging to the map is loaded.  In this case the path element
   containing $ORIGIN is left out.  */
static char *
expand_dynamic_string_token (struct link_map *l, const char *s, int is_path)
{
  /* We make two runs over the string.  First we determine how large the
     resulting string is and then we copy it over.  Since this is no
     frequently executed operation we are looking here not for performance
     but rather for code size.  */
  size_t cnt;
  size_t total;
  char *result;

  /* Determine the number of DST elements.  */
  cnt = DL_DST_COUNT (s, is_path);

  /* If we do not have to replace anything simply copy the string.  */
  if (__glibc_likely (cnt == 0))
    return __strdup (s);

  /* Determine the length of the substituted string.  */
  total = DL_DST_REQUIRED (l, s, strlen (s), cnt);

  /* Allocate the necessary memory.  */
  result = (char *) malloc (total + 1);
  if (result == NULL)
    return NULL;

  return _dl_dst_substitute (l, s, result, is_path);
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
      _dl_signal_error (ENOMEM, name, NULL, N_("cannot allocate name record"));
      return;
    }
  /* The object should have a libname set from _dl_new_object.  */
  assert (lastp != NULL);

  newname->name = memcpy (newname + 1, name, name_len);
  newname->next = NULL;
  newname->dont_free = 0;
  lastp->next = newname;
}

/* Standard search directories.  */
static struct r_search_path_struct rtld_search_dirs attribute_relro;

static size_t max_dirnamelen;

static struct r_search_path_elem **
fillin_rpath (char *rpath, struct r_search_path_elem **result, const char *sep,
	      int check_trusted, const char *what, const char *where,
	      struct link_map *l)
{
  char *cp;
  size_t nelems = 0;
  char *to_free;

  while ((cp = __strsep (&rpath, sep)) != NULL)
    {
      struct r_search_path_elem *dirp;

      to_free = cp = expand_dynamic_string_token (l, cp, 1);

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

      /* Make sure we don't use untrusted directories if we run SUID.  */
      if (__glibc_unlikely (check_trusted) && !is_trusted_path (cp, len))
	{
	  free (to_free);
	  continue;
	}

      /* See if this directory is already known.  */
      for (dirp = GL(dl_all_dirs); dirp != NULL; dirp = dirp->next)
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
	    _dl_signal_error (ENOMEM, NULL, NULL,
			      N_("cannot create cache for search path"));

	  dirp->dirname = ((char *) dirp + sizeof (*dirp)
			   + ncapstr * sizeof (enum r_dir_status));
	  *((char *) __mempcpy ((char *) dirp->dirname, cp, len)) = '\0';
	  dirp->dirnamelen = len;

	  if (len > max_dirnamelen)
	    max_dirnamelen = len;

	  /* We have to make sure all the relative directories are
	     never ignored.  The current directory might change and
	     all our saved information would be void.  */
	  init_val = cp[0] != '/' ? existing : unknown;
	  for (cnt = 0; cnt < ncapstr; ++cnt)
	    dirp->status[cnt] = init_val;

	  dirp->what = what;
	  if (__glibc_likely (where != NULL))
	    dirp->where = memcpy ((char *) dirp + sizeof (*dirp) + len + 1
				  + (ncapstr * sizeof (enum r_dir_status)),
				  where, where_len);
	  else
	    dirp->where = NULL;

	  dirp->next = GL(dl_all_dirs);
	  GL(dl_all_dirs) = dirp;

	  /* Put it in the result array.  */
	  result[nelems++] = dirp;
	}
      free (to_free);
    }

  /* Terminate the array.  */
  result[nelems] = NULL;

  return result;
}


static bool
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
  /* Initialize to please the compiler.  */
  const char *errstring = NULL;

  /* First see whether we must forget the RUNPATH and RPATH from this
     object.  */
  if (__glibc_unlikely (GLRO(dl_inhibit_rpath) != NULL)
      && !__libc_enable_secure)
    {
      const char *inhp = GLRO(dl_inhibit_rpath);

      do
	{
	  const char *wp = where;

	  while (*inhp == *wp && *wp != '\0')
	    {
	      ++inhp;
	      ++wp;
	    }

	  if (*wp == '\0' && (*inhp == '\0' || *inhp == ':'))
	    {
	      /* This object is on the list of objects for which the
		 RUNPATH and RPATH must not be used.  */
	      sps->dirs = (void *) -1;
	      return false;
	    }

	  while (*inhp != '\0')
	    if (*inhp++ == ':')
	      break;
	}
      while (*inhp != '\0');
    }

  /* Make a writable copy.  */
  copy = __strdup (rpath);
  if (copy == NULL)
    {
      errstring = N_("cannot create RUNPATH/RPATH copy");
      goto signal_error;
    }

  /* Ignore empty rpaths.  */
  if (*copy == 0)
    {
      free (copy);
      sps->dirs = (struct r_search_path_elem **) -1;
      return false;
    }

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
    {
      free (copy);
      errstring = N_("cannot create cache for search path");
    signal_error:
      _dl_signal_error (ENOMEM, NULL, NULL, errstring);
    }

  fillin_rpath (copy, result, ":", 0, what, where, l);

  /* Free the copied RPATH string.  `fillin_rpath' make own copies if
     necessary.  */
  free (copy);

  sps->dirs = result;
  /* The caller will change this value if we haven't used a real malloc.  */
  sps->malloced = 1;
  return true;
}

/* Make sure cached path information is stored in *SP
   and return true if there are any paths to search there.  */
static bool
cache_rpath (struct link_map *l,
	     struct r_search_path_struct *sp,
	     int tag,
	     const char *what)
{
  if (sp->dirs == (void *) -1)
    return false;

  if (sp->dirs != NULL)
    return true;

  if (l->l_info[tag] == NULL)
    {
      /* There is no path.  */
      sp->dirs = (void *) -1;
      return false;
    }

  /* Make sure the cache information is available.  */
  return decompose_rpath (sp, (const char *) (D_PTR (l, l_info[DT_STRTAB])
					      + l->l_info[tag]->d_un.d_val),
			  l, what);
}


void
internal_function
_dl_init_paths (const char *llp)
{
  size_t idx;
  const char *strp;
  struct r_search_path_elem *pelem, **aelem;
  size_t round_size;
  struct link_map __attribute__ ((unused)) *l = NULL;
  /* Initialize to please the compiler.  */
  const char *errstring = NULL;

  /* Fill in the information about the application's RPATH and the
     directories addressed by the LD_LIBRARY_PATH environment variable.  */

  /* Get the capabilities.  */
  capstr = _dl_important_hwcaps (GLRO(dl_platform), GLRO(dl_platformlen),
				 &ncapstr, &max_capstrlen);

  /* First set up the rest of the default search directory entries.  */
  aelem = rtld_search_dirs.dirs = (struct r_search_path_elem **)
    malloc ((nsystem_dirs_len + 1) * sizeof (struct r_search_path_elem *));
  if (rtld_search_dirs.dirs == NULL)
    {
      errstring = N_("cannot create search path array");
    signal_error:
      _dl_signal_error (ENOMEM, NULL, NULL, errstring);
    }

  round_size = ((2 * sizeof (struct r_search_path_elem) - 1
		 + ncapstr * sizeof (enum r_dir_status))
		/ sizeof (struct r_search_path_elem));

  rtld_search_dirs.dirs[0] = (struct r_search_path_elem *)
    malloc ((sizeof (system_dirs) / sizeof (system_dirs[0]))
	    * round_size * sizeof (struct r_search_path_elem));
  if (rtld_search_dirs.dirs[0] == NULL)
    {
      errstring = N_("cannot create cache for search path");
      goto signal_error;
    }

  rtld_search_dirs.malloced = 0;
  pelem = GL(dl_all_dirs) = rtld_search_dirs.dirs[0];
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
  l = GL(dl_ns)[LM_ID_BASE]._ns_loaded;
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
	  /* During rtld init the memory is allocated by the stub malloc,
	     prevent any attempt to free it by the normal malloc.  */
	  l->l_runpath_dirs.malloced = 0;

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
	      /* During rtld init the memory is allocated by the stub
		 malloc, prevent any attempt to free it by the normal
		 malloc.  */
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
      char *llp_tmp;

#ifdef SHARED
      /* Expand DSTs.  */
      size_t cnt = DL_DST_COUNT (llp, 1);
      if (__glibc_likely (cnt == 0))
	llp_tmp = strdupa (llp);
      else
	{
	  /* Determine the length of the substituted string.  */
	  size_t total = DL_DST_REQUIRED (l, llp, strlen (llp), cnt);

	  /* Allocate the necessary memory.  */
	  llp_tmp = (char *) alloca (total + 1);
	  llp_tmp = _dl_dst_substitute (l, llp, llp_tmp, 1);
	}
#else
      llp_tmp = strdupa (llp);
#endif

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
	{
	  errstring = N_("cannot create cache for search path");
	  goto signal_error;
	}

      (void) fillin_rpath (llp_tmp, env_path_list.dirs, ":;",
			   __libc_enable_secure, "LD_LIBRARY_PATH",
			   NULL, l);

      if (env_path_list.dirs[0] == NULL)
	{
	  free (env_path_list.dirs);
	  env_path_list.dirs = (void *) -1;
	}

      env_path_list.malloced = 0;
    }
  else
    env_path_list.dirs = (void *) -1;
}


static void
__attribute__ ((noreturn, noinline))
lose (int code, int fd, const char *name, char *realname, struct link_map *l,
      const char *msg, struct r_debug *r, Lmid_t nsid)
{
  /* The file might already be closed.  */
  if (fd != -1)
    (void) __close (fd);
  if (l != NULL && l->l_origin != (char *) -1l)
    free ((char *) l->l_origin);
  free (l);
  free (realname);

  if (r != NULL)
    {
      r->r_state = RT_CONSISTENT;
      _dl_debug_state ();
      LIBC_PROBE (map_failed, 2, nsid, r);
    }

  _dl_signal_error (code, name, NULL, msg);
}


/* Map in the shared object NAME, actually located in REALNAME, and already
   opened on FD.  */

#ifndef EXTERNAL_MAP_FROM_FD
static
#endif
struct link_map *
_dl_map_object_from_fd (const char *name, const char *origname, int fd,
			struct filebuf *fbp, char *realname,
			struct link_map *loader, int l_type, int mode,
			void **stack_endp, Lmid_t nsid)
{
  struct link_map *l = NULL;
  const ElfW(Ehdr) *header;
  const ElfW(Phdr) *phdr;
  const ElfW(Phdr) *ph;
  size_t maplength;
  int type;
  /* Initialize to keep the compiler happy.  */
  const char *errstring = NULL;
  int errval = 0;
  struct r_debug *r = _dl_debug_initialize (0, nsid);
  bool make_consistent = false;

  /* Get file information.  */
  struct r_file_id id;
  if (__glibc_unlikely (!_dl_get_file_id (fd, &id)))
    {
      errstring = N_("cannot stat shared object");
    call_lose_errno:
      errval = errno;
    call_lose:
      lose (errval, fd, name, realname, l, errstring,
	    make_consistent ? r : NULL, nsid);
    }

  /* Look again to see if the real name matched another already loaded.  */
  for (l = GL(dl_ns)[nsid]._ns_loaded; l != NULL; l = l->l_next)
    if (!l->l_removed && _dl_file_id_match_p (&l->l_file_id, &id))
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

#ifdef SHARED
  /* When loading into a namespace other than the base one we must
     avoid loading ld.so since there can only be one copy.  Ever.  */
  if (__glibc_unlikely (nsid != LM_ID_BASE)
      && (_dl_file_id_match_p (&id, &GL(dl_rtld_map).l_file_id)
	  || _dl_name_match_p (name, &GL(dl_rtld_map))))
    {
      /* This is indeed ld.so.  Create a new link_map which refers to
	 the real one for almost everything.  */
      l = _dl_new_object (realname, name, l_type, loader, mode, nsid);
      if (l == NULL)
	goto fail_new;

      /* Refer to the real descriptor.  */
      l->l_real = &GL(dl_rtld_map);

      /* No need to bump the refcount of the real object, ld.so will
	 never be unloaded.  */
      __close (fd);

      /* Add the map for the mirrored object to the object list.  */
      _dl_add_to_namespace_list (l, nsid);

      return l;
    }
#endif

  if (mode & RTLD_NOLOAD)
    {
      /* We are not supposed to load the object unless it is already
	 loaded.  So return now.  */
      free (realname);
      __close (fd);
      return NULL;
    }

  /* Print debugging message.  */
  if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_FILES))
    _dl_debug_printf ("file=%s [%lu];  generating link map\n", name, nsid);

  /* This is the ELF header.  We read it in `open_verify'.  */
  header = (void *) fbp->buf;

#ifndef MAP_ANON
# define MAP_ANON 0
  if (_dl_zerofd == -1)
    {
      _dl_zerofd = _dl_sysdep_open_zero_fill ();
      if (_dl_zerofd == -1)
	{
	  free (realname);
	  __close (fd);
	  _dl_signal_error (errno, NULL, NULL,
			    N_("cannot open zero fill device"));
	}
    }
#endif

  /* Signal that we are going to add new objects.  */
  if (r->r_state == RT_CONSISTENT)
    {
#ifdef SHARED
      /* Auditing checkpoint: we are going to add new objects.  */
      if ((mode & __RTLD_AUDIT) == 0
	  && __glibc_unlikely (GLRO(dl_naudit) > 0))
	{
	  struct link_map *head = GL(dl_ns)[nsid]._ns_loaded;
	  /* Do not call the functions for any auditing object.  */
	  if (head->l_auditing == 0)
	    {
	      struct audit_ifaces *afct = GLRO(dl_audit);
	      for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
		{
		  if (afct->activity != NULL)
		    afct->activity (&head->l_audit[cnt].cookie, LA_ACT_ADD);

		  afct = afct->next;
		}
	    }
	}
#endif

      /* Notify the debugger we have added some objects.  We need to
	 call _dl_debug_initialize in a static program in case dynamic
	 linking has not been used before.  */
      r->r_state = RT_ADD;
      _dl_debug_state ();
      LIBC_PROBE (map_start, 2, nsid, r);
      make_consistent = true;
    }
  else
    assert (r->r_state == RT_ADD);

  /* Enter the new object in the list of loaded objects.  */
  l = _dl_new_object (realname, name, l_type, loader, mode, nsid);
  if (__glibc_unlikely (l == NULL))
    {
#ifdef SHARED
    fail_new:
#endif
      errstring = N_("cannot create shared object descriptor");
      goto call_lose_errno;
    }

  /* Extract the remaining details we need from the ELF header
     and then read in the program header table.  */
  l->l_entry = header->e_entry;
  type = header->e_type;
  l->l_phnum = header->e_phnum;

  maplength = header->e_phnum * sizeof (ElfW(Phdr));
  if (header->e_phoff + maplength <= (size_t) fbp->len)
    phdr = (void *) (fbp->buf + header->e_phoff);
  else
    {
      phdr = alloca (maplength);
      __lseek (fd, header->e_phoff, SEEK_SET);
      if ((size_t) __libc_read (fd, (void *) phdr, maplength) != maplength)
	{
	  errstring = N_("cannot read file data");
	  goto call_lose_errno;
	}
    }

   /* On most platforms presume that PT_GNU_STACK is absent and the stack is
    * executable.  Other platforms default to a nonexecutable stack and don't
    * need PT_GNU_STACK to do so.  */
   uint_fast16_t stack_flags = DEFAULT_STACK_PERMS;

  {
    /* Scan the program header table, collecting its load commands.  */
    struct loadcmd loadcmds[l->l_phnum];
    size_t nloadcmds = 0;
    bool has_holes = false;

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
	  if (__glibc_unlikely ((ph->p_align & (GLRO(dl_pagesize) - 1)) != 0))
	    {
	      errstring = N_("ELF load command alignment not page-aligned");
	      goto call_lose;
	    }
	  if (__glibc_unlikely (((ph->p_vaddr - ph->p_offset)
				 & (ph->p_align - 1)) != 0))
	    {
	      errstring
		= N_("ELF load command address/offset not properly aligned");
	      goto call_lose;
	    }

	  struct loadcmd *c = &loadcmds[nloadcmds++];
	  c->mapstart = ALIGN_DOWN (ph->p_vaddr, GLRO(dl_pagesize));
	  c->mapend = ALIGN_UP (ph->p_vaddr + ph->p_filesz, GLRO(dl_pagesize));
	  c->dataend = ph->p_vaddr + ph->p_filesz;
	  c->allocend = ph->p_vaddr + ph->p_memsz;
	  c->mapoff = ALIGN_DOWN (ph->p_offset, GLRO(dl_pagesize));

	  /* Determine whether there is a gap between the last segment
	     and this one.  */
	  if (nloadcmds > 1 && c[-1].mapend != c->mapstart)
	    has_holes = true;

	  /* Optimize a common case.  */
#if (PF_R | PF_W | PF_X) == 7 && (PROT_READ | PROT_WRITE | PROT_EXEC) == 7
	  c->prot = (PF_TO_PROT
		     >> ((ph->p_flags & (PF_R | PF_W | PF_X)) * 4)) & 0xf;
#else
	  c->prot = 0;
	  if (ph->p_flags & PF_R)
	    c->prot |= PROT_READ;
	  if (ph->p_flags & PF_W)
	    c->prot |= PROT_WRITE;
	  if (ph->p_flags & PF_X)
	    c->prot |= PROT_EXEC;
#endif
	  break;

	case PT_TLS:
	  if (ph->p_memsz == 0)
	    /* Nothing to do for an empty segment.  */
	    break;

	  l->l_tls_blocksize = ph->p_memsz;
	  l->l_tls_align = ph->p_align;
	  if (ph->p_align == 0)
	    l->l_tls_firstbyte_offset = 0;
	  else
	    l->l_tls_firstbyte_offset = ph->p_vaddr & (ph->p_align - 1);
	  l->l_tls_initimage_size = ph->p_filesz;
	  /* Since we don't know the load address yet only store the
	     offset.  We will adjust it later.  */
	  l->l_tls_initimage = (void *) ph->p_vaddr;

	  /* If not loading the initial set of shared libraries,
	     check whether we should permit loading a TLS segment.  */
	  if (__glibc_likely (l->l_type == lt_library)
	      /* If GL(dl_tls_dtv_slotinfo_list) == NULL, then rtld.c did
		 not set up TLS data structures, so don't use them now.  */
	      || __glibc_likely (GL(dl_tls_dtv_slotinfo_list) != NULL))
	    {
	      /* Assign the next available module ID.  */
	      l->l_tls_modid = _dl_next_tls_modid ();
	      break;
	    }

#ifdef SHARED
	  /* We are loading the executable itself when the dynamic
	     linker was executed directly.  The setup will happen
	     later.  Otherwise, the TLS data structures are already
	     initialized, and we assigned a TLS modid above.  */
	  assert (l->l_prev == NULL || (mode & __RTLD_AUDIT) != 0);
#else
	  assert (false && "TLS not initialized in static application");
#endif
	  break;

	case PT_GNU_STACK:
	  stack_flags = ph->p_flags;
	  break;

	case PT_GNU_RELRO:
	  l->l_relro_addr = ph->p_vaddr;
	  l->l_relro_size = ph->p_memsz;
	  break;
	}

    if (__glibc_unlikely (nloadcmds == 0))
      {
	/* This only happens for a bogus object that will be caught with
	   another error below.  But we don't want to go through the
	   calculations below using NLOADCMDS - 1.  */
	errstring = N_("object file has no loadable segments");
	goto call_lose;
      }

    if (__glibc_unlikely (type != ET_DYN)
	&& __glibc_unlikely ((mode & __RTLD_OPENEXEC) == 0))
      {
	/* This object is loaded at a fixed address.  This must never
	   happen for objects loaded with dlopen.  */
	errstring = N_("cannot dynamically load executable");
	goto call_lose;
      }

    /* Length of the sections to be loaded.  */
    maplength = loadcmds[nloadcmds - 1].allocend - loadcmds[0].mapstart;

    /* Now process the load commands and map segments into memory.
       This is responsible for filling in:
       l_map_start, l_map_end, l_addr, l_contiguous, l_text_end, l_phdr
     */
    errstring = _dl_map_segments (l, fd, header, type, loadcmds, nloadcmds,
				  maplength, has_holes, loader);
    if (__glibc_unlikely (errstring != NULL))
      goto call_lose;
  }

  if (l->l_ld == 0)
    {
      if (__glibc_unlikely (type == ET_DYN))
	{
	  errstring = N_("object file has no dynamic section");
	  goto call_lose;
	}
    }
  else
    l->l_ld = (ElfW(Dyn) *) ((ElfW(Addr)) l->l_ld + l->l_addr);

  elf_get_dynamic_info (l, NULL);

  /* Make sure we are not dlopen'ing an object that has the
     DF_1_NOOPEN flag set.  */
  if (__glibc_unlikely (l->l_flags_1 & DF_1_NOOPEN)
      && (mode & __RTLD_DLOPEN))
    {
      /* We are not supposed to load this object.  Free all resources.  */
      _dl_unmap_segments (l);

      if (!l->l_libname->dont_free)
	free (l->l_libname);

      if (l->l_phdr_allocated)
	free ((void *) l->l_phdr);

      errstring = N_("shared object cannot be dlopen()ed");
      goto call_lose;
    }

  if (l->l_phdr == NULL)
    {
      /* The program header is not contained in any of the segments.
	 We have to allocate memory ourself and copy it over from out
	 temporary place.  */
      ElfW(Phdr) *newp = (ElfW(Phdr) *) malloc (header->e_phnum
						* sizeof (ElfW(Phdr)));
      if (newp == NULL)
	{
	  errstring = N_("cannot allocate memory for program header");
	  goto call_lose_errno;
	}

      l->l_phdr = memcpy (newp, phdr,
			  (header->e_phnum * sizeof (ElfW(Phdr))));
      l->l_phdr_allocated = 1;
    }
  else
    /* Adjust the PT_PHDR value by the runtime load address.  */
    l->l_phdr = (ElfW(Phdr) *) ((ElfW(Addr)) l->l_phdr + l->l_addr);

  if (__glibc_unlikely ((stack_flags &~ GL(dl_stack_flags)) & PF_X))
    {
      if (__glibc_unlikely (__check_caller (RETURN_ADDRESS (0), allow_ldso) != 0))
	{
	  errstring = N_("invalid caller");
	  goto call_lose;
	}

      /* The stack is presently not executable, but this module
	 requires that it be executable.  We must change the
	 protection of the variable which contains the flags used in
	 the mprotect calls.  */
#ifdef SHARED
      if ((mode & (__RTLD_DLOPEN | __RTLD_AUDIT)) == __RTLD_DLOPEN)
	{
	  const uintptr_t p = (uintptr_t) &__stack_prot & -GLRO(dl_pagesize);
	  const size_t s = (uintptr_t) (&__stack_prot + 1) - p;

	  struct link_map *const m = &GL(dl_rtld_map);
	  const uintptr_t relro_end = ((m->l_addr + m->l_relro_addr
					+ m->l_relro_size)
				       & -GLRO(dl_pagesize));
	  if (__glibc_likely (p + s <= relro_end))
	    {
	      /* The variable lies in the region protected by RELRO.  */
	      if (__mprotect ((void *) p, s, PROT_READ|PROT_WRITE) < 0)
		{
		  errstring = N_("cannot change memory protections");
		  goto call_lose_errno;
		}
	      __stack_prot |= PROT_READ|PROT_WRITE|PROT_EXEC;
	      __mprotect ((void *) p, s, PROT_READ);
	    }
	  else
	    __stack_prot |= PROT_READ|PROT_WRITE|PROT_EXEC;
	}
      else
#endif
	__stack_prot |= PROT_READ|PROT_WRITE|PROT_EXEC;

#ifdef check_consistency
      check_consistency ();
#endif

      errval = (*GL(dl_make_stack_executable_hook)) (stack_endp);
      if (errval)
	{
	  errstring = N_("\
cannot enable executable stack as shared object requires");
	  goto call_lose;
	}
    }

  /* Adjust the address of the TLS initialization image.  */
  if (l->l_tls_initimage != NULL)
    l->l_tls_initimage = (char *) l->l_tls_initimage + l->l_addr;

  /* We are done mapping in the file.  We no longer need the descriptor.  */
  if (__glibc_unlikely (__close (fd) != 0))
    {
      errstring = N_("cannot close file descriptor");
      goto call_lose_errno;
    }
  /* Signal that we closed the file.  */
  fd = -1;

  /* If this is ET_EXEC, we should have loaded it as lt_executable.  */
  assert (type != ET_EXEC || l->l_type == lt_executable);

  l->l_entry += l->l_addr;

  if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_FILES))
    _dl_debug_printf ("\
  dynamic: 0x%0*lx  base: 0x%0*lx   size: 0x%0*Zx\n\
    entry: 0x%0*lx  phdr: 0x%0*lx  phnum:   %*u\n\n",
			   (int) sizeof (void *) * 2,
			   (unsigned long int) l->l_ld,
			   (int) sizeof (void *) * 2,
			   (unsigned long int) l->l_addr,
			   (int) sizeof (void *) * 2, maplength,
			   (int) sizeof (void *) * 2,
			   (unsigned long int) l->l_entry,
			   (int) sizeof (void *) * 2,
			   (unsigned long int) l->l_phdr,
			   (int) sizeof (void *) * 2, l->l_phnum);

  /* Set up the symbol hash table.  */
  _dl_setup_hash (l);

  /* If this object has DT_SYMBOLIC set modify now its scope.  We don't
     have to do this for the main map.  */
  if ((mode & RTLD_DEEPBIND) == 0
      && __glibc_unlikely (l->l_info[DT_SYMBOLIC] != NULL)
      && &l->l_searchlist != l->l_scope[0])
    {
      /* Create an appropriate searchlist.  It contains only this map.
	 This is the definition of DT_SYMBOLIC in SysVr4.  */
      l->l_symbolic_searchlist.r_list[0] = l;
      l->l_symbolic_searchlist.r_nlist = 1;

      /* Now move the existing entries one back.  */
      memmove (&l->l_scope[1], &l->l_scope[0],
	       (l->l_scope_max - 1) * sizeof (l->l_scope[0]));

      /* Now add the new entry.  */
      l->l_scope[0] = &l->l_symbolic_searchlist;
    }

  /* Remember whether this object must be initialized first.  */
  if (l->l_flags_1 & DF_1_INITFIRST)
    GL(dl_initfirst) = l;

  /* Finally the file information.  */
  l->l_file_id = id;

#ifdef SHARED
  /* When auditing is used the recorded names might not include the
     name by which the DSO is actually known.  Add that as well.  */
  if (__glibc_unlikely (origname != NULL))
    add_name_to_object (l, origname);
#else
  /* Audit modules only exist when linking is dynamic so ORIGNAME
     cannot be non-NULL.  */
  assert (origname == NULL);
#endif

  /* When we profile the SONAME might be needed for something else but
     loading.  Add it right away.  */
  if (__glibc_unlikely (GLRO(dl_profile) != NULL)
      && l->l_info[DT_SONAME] != NULL)
    add_name_to_object (l, ((const char *) D_PTR (l, l_info[DT_STRTAB])
			    + l->l_info[DT_SONAME]->d_un.d_val));

#ifdef DL_AFTER_LOAD
  DL_AFTER_LOAD (l);
#endif

  /* Now that the object is fully initialized add it to the object list.  */
  _dl_add_to_namespace_list (l, nsid);

#ifdef SHARED
  /* Auditing checkpoint: we have a new object.  */
  if (__glibc_unlikely (GLRO(dl_naudit) > 0)
      && !GL(dl_ns)[l->l_ns]._ns_loaded->l_auditing)
    {
      struct audit_ifaces *afct = GLRO(dl_audit);
      for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
	{
	  if (afct->objopen != NULL)
	    {
	      l->l_audit[cnt].bindflags
		= afct->objopen (l, nsid, &l->l_audit[cnt].cookie);

	      l->l_audit_any_plt |= l->l_audit[cnt].bindflags != 0;
	    }

	  afct = afct->next;
	}
    }
#endif

  return l;
}

/* Print search path.  */
static void
print_search_path (struct r_search_path_elem **list,
		   const char *what, const char *name)
{
  char buf[max_dirnamelen + max_capstrlen];
  int first = 1;

  _dl_debug_printf (" search path=");

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

	    _dl_debug_printf_c (first ? "%s" : ":%s", buf);
	    first = 0;
	  }

      ++list;
    }

  if (name != NULL)
    _dl_debug_printf_c ("\t\t(%s from file %s)\n", what,
			DSO_FILENAME (name));
  else
    _dl_debug_printf_c ("\t\t(%s)\n", what);
}

/* Open a file and verify it is an ELF file for this architecture.  We
   ignore only ELF files for other architectures.  Non-ELF files and
   ELF files with different header information cause fatal errors since
   this could mean there is something wrong in the installation and the
   user might want to know about this.

   If FD is not -1, then the file is already open and FD refers to it.
   In that case, FD is consumed for both successful and error returns.  */
static int
open_verify (const char *name, int fd,
             struct filebuf *fbp, struct link_map *loader,
	     int whatcode, int mode, bool *found_other_class, bool free_name)
{
  /* This is the expected ELF header.  */
#define ELF32_CLASS ELFCLASS32
#define ELF64_CLASS ELFCLASS64
#ifndef VALID_ELF_HEADER
# define VALID_ELF_HEADER(hdr,exp,size)	(memcmp (hdr, exp, size) == 0)
# define VALID_ELF_OSABI(osabi)		(osabi == ELFOSABI_SYSV)
# define VALID_ELF_ABIVERSION(osabi,ver) (ver == 0)
#elif defined MORE_ELF_HEADER_DATA
  MORE_ELF_HEADER_DATA;
#endif
  static const unsigned char expected[EI_NIDENT] =
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
  static const struct
  {
    ElfW(Word) vendorlen;
    ElfW(Word) datalen;
    ElfW(Word) type;
    char vendor[4];
  } expected_note = { 4, 16, 1, "GNU" };
  /* Initialize it to make the compiler happy.  */
  const char *errstring = NULL;
  int errval = 0;

#ifdef SHARED
  /* Give the auditing libraries a chance.  */
  if (__glibc_unlikely (GLRO(dl_naudit) > 0) && whatcode != 0
      && loader->l_auditing == 0)
    {
      const char *original_name = name;
      struct audit_ifaces *afct = GLRO(dl_audit);
      for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
	{
	  if (afct->objsearch != NULL)
	    {
	      name = afct->objsearch (name, &loader->l_audit[cnt].cookie,
				      whatcode);
	      if (name == NULL)
		/* Ignore the path.  */
		return -1;
	    }

	  afct = afct->next;
	}

      if (fd != -1 && name != original_name && strcmp (name, original_name))
        {
          /* An audit library changed what we're supposed to open,
             so FD no longer matches it.  */
          __close (fd);
          fd = -1;
        }
    }
#endif

  if (fd == -1)
    /* Open the file.  We always open files read-only.  */
    fd = __open (name, O_RDONLY | O_CLOEXEC);

  if (fd != -1)
    {
      ElfW(Ehdr) *ehdr;
      ElfW(Phdr) *phdr, *ph;
      ElfW(Word) *abi_note;
      unsigned int osversion;
      size_t maplength;

      /* We successfully opened the file.  Now verify it is a file
	 we can use.  */
      __set_errno (0);
      fbp->len = 0;
      assert (sizeof (fbp->buf) > sizeof (ElfW(Ehdr)));
      /* Read in the header.  */
      do
	{
	  ssize_t retlen = __libc_read (fd, fbp->buf + fbp->len,
					sizeof (fbp->buf) - fbp->len);
	  if (retlen <= 0)
	    break;
	  fbp->len += retlen;
	}
      while (__glibc_unlikely (fbp->len < sizeof (ElfW(Ehdr))));

      /* This is where the ELF header is loaded.  */
      ehdr = (ElfW(Ehdr) *) fbp->buf;

      /* Now run the tests.  */
      if (__glibc_unlikely (fbp->len < (ssize_t) sizeof (ElfW(Ehdr))))
	{
	  errval = errno;
	  errstring = (errval == 0
		       ? N_("file too short") : N_("cannot read file data"));
	call_lose:
	  if (free_name)
	    {
	      char *realname = (char *) name;
	      name = strdupa (realname);
	      free (realname);
	    }
	  lose (errval, fd, name, NULL, NULL, errstring, NULL, 0);
	}

      /* See whether the ELF header is what we expect.  */
      if (__glibc_unlikely (! VALID_ELF_HEADER (ehdr->e_ident, expected,
						EI_ABIVERSION)
			    || !VALID_ELF_ABIVERSION (ehdr->e_ident[EI_OSABI],
						      ehdr->e_ident[EI_ABIVERSION])
			    || memcmp (&ehdr->e_ident[EI_PAD],
				       &expected[EI_PAD],
				       EI_NIDENT - EI_PAD) != 0))
	{
	  /* Something is wrong.  */
	  const Elf32_Word *magp = (const void *) ehdr->e_ident;
	  if (*magp !=
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
	    errstring = N_("invalid ELF header");
	  else if (ehdr->e_ident[EI_CLASS] != ELFW(CLASS))
	    {
	      /* This is not a fatal error.  On architectures where
		 32-bit and 64-bit binaries can be run this might
		 happen.  */
	      *found_other_class = true;
	      goto close_and_out;
	    }
	  else if (ehdr->e_ident[EI_DATA] != byteorder)
	    {
	      if (BYTE_ORDER == BIG_ENDIAN)
		errstring = N_("ELF file data encoding not big-endian");
	      else
		errstring = N_("ELF file data encoding not little-endian");
	    }
	  else if (ehdr->e_ident[EI_VERSION] != EV_CURRENT)
	    errstring
	      = N_("ELF file version ident does not match current one");
	  /* XXX We should be able so set system specific versions which are
	     allowed here.  */
	  else if (!VALID_ELF_OSABI (ehdr->e_ident[EI_OSABI]))
	    errstring = N_("ELF file OS ABI invalid");
	  else if (!VALID_ELF_ABIVERSION (ehdr->e_ident[EI_OSABI],
					  ehdr->e_ident[EI_ABIVERSION]))
	    errstring = N_("ELF file ABI version invalid");
	  else if (memcmp (&ehdr->e_ident[EI_PAD], &expected[EI_PAD],
			   EI_NIDENT - EI_PAD) != 0)
	    errstring = N_("nonzero padding in e_ident");
	  else
	    /* Otherwise we don't know what went wrong.  */
	    errstring = N_("internal error");

	  goto call_lose;
	}

      if (__glibc_unlikely (ehdr->e_version != EV_CURRENT))
	{
	  errstring = N_("ELF file version does not match current one");
	  goto call_lose;
	}
      if (! __glibc_likely (elf_machine_matches_host (ehdr)))
	goto close_and_out;
      else if (__glibc_unlikely (ehdr->e_type != ET_DYN
				 && ehdr->e_type != ET_EXEC))
	{
	  errstring = N_("only ET_DYN and ET_EXEC can be loaded");
	  goto call_lose;
	}
      else if (__glibc_unlikely (ehdr->e_type == ET_EXEC
				 && (mode & __RTLD_OPENEXEC) == 0))
	{
	  /* BZ #16634. It is an error to dlopen ET_EXEC (unless
	     __RTLD_OPENEXEC is explicitly set).  We return error here
	     so that code in _dl_map_object_from_fd does not try to set
	     l_tls_modid for this module.  */

	  errstring = N_("cannot dynamically load executable");
	  goto call_lose;
	}
      else if (__glibc_unlikely (ehdr->e_phentsize != sizeof (ElfW(Phdr))))
	{
	  errstring = N_("ELF file's phentsize not the expected size");
	  goto call_lose;
	}

      maplength = ehdr->e_phnum * sizeof (ElfW(Phdr));
      if (ehdr->e_phoff + maplength <= (size_t) fbp->len)
	phdr = (void *) (fbp->buf + ehdr->e_phoff);
      else
	{
	  phdr = alloca (maplength);
	  __lseek (fd, ehdr->e_phoff, SEEK_SET);
	  if ((size_t) __libc_read (fd, (void *) phdr, maplength) != maplength)
	    {
	    read_error:
	      errval = errno;
	      errstring = N_("cannot read file data");
	      goto call_lose;
	    }
	}

      if (__glibc_unlikely (elf_machine_reject_phdr_p
			    (phdr, ehdr->e_phnum, fbp->buf, fbp->len,
			     loader, fd)))
	goto close_and_out;

      /* Check .note.ABI-tag if present.  */
      for (ph = phdr; ph < &phdr[ehdr->e_phnum]; ++ph)
	if (ph->p_type == PT_NOTE && ph->p_filesz >= 32 && ph->p_align >= 4)
	  {
	    ElfW(Addr) size = ph->p_filesz;

	    if (ph->p_offset + size <= (size_t) fbp->len)
	      abi_note = (void *) (fbp->buf + ph->p_offset);
	    else
	      {
		abi_note = alloca (size);
		__lseek (fd, ph->p_offset, SEEK_SET);
		if (__libc_read (fd, (void *) abi_note, size) != size)
		  goto read_error;
	      }

	    while (memcmp (abi_note, &expected_note, sizeof (expected_note)))
	      {
#define ROUND(len) (((len) + sizeof (ElfW(Word)) - 1) & -sizeof (ElfW(Word)))
		ElfW(Addr) note_size = 3 * sizeof (ElfW(Word))
				       + ROUND (abi_note[0])
				       + ROUND (abi_note[1]);

		if (size - 32 < note_size)
		  {
		    size = 0;
		    break;
		  }
		size -= note_size;
		abi_note = (void *) abi_note + note_size;
	      }

	    if (size == 0)
	      continue;

	    osversion = (abi_note[5] & 0xff) * 65536
			+ (abi_note[6] & 0xff) * 256
			+ (abi_note[7] & 0xff);
	    if (abi_note[4] != __ABI_TAG_OS
		|| (GLRO(dl_osversion) && GLRO(dl_osversion) < osversion))
	      {
	      close_and_out:
		__close (fd);
		__set_errno (ENOENT);
		fd = -1;
	      }

	    break;
	  }
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
open_path (const char *name, size_t namelen, int mode,
	   struct r_search_path_struct *sps, char **realname,
	   struct filebuf *fbp, struct link_map *loader, int whatcode,
	   bool *found_other_class)
{
  struct r_search_path_elem **dirs = sps->dirs;
  char *buf;
  int fd = -1;
  const char *current_what = NULL;
  int any = 0;

  if (__glibc_unlikely (dirs == NULL))
    /* We're called before _dl_init_paths when loading the main executable
       given on the command line when rtld is run directly.  */
    return -1;

  buf = alloca (max_dirnamelen + max_capstrlen + namelen);
  do
    {
      struct r_search_path_elem *this_dir = *dirs;
      size_t buflen = 0;
      size_t cnt;
      char *edp;
      int here_any = 0;
      int err;

      /* If we are debugging the search for libraries print the path
	 now if it hasn't happened now.  */
      if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_LIBS)
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
	    ((char *) __mempcpy (__mempcpy (edp, capstr[cnt].str,
					    capstr[cnt].len),
				 name, namelen)
	     - buf);

	  /* Print name we try if this is wanted.  */
	  if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_LIBS))
	    _dl_debug_printf ("  trying file=%s\n", buf);

	  fd = open_verify (buf, -1, fbp, loader, whatcode, mode,
			    found_other_class, false);
	  if (this_dir->status[cnt] == unknown)
	    {
	      if (fd != -1)
		this_dir->status[cnt] = existing;
	      /* Do not update the directory information when loading
		 auditing code.  We must try to disturb the program as
		 little as possible.  */
	      else if (loader == NULL
		       || GL(dl_ns)[loader->l_ns]._ns_loaded->l_auditing == 0)
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
	  here_any |= this_dir->status[cnt] != nonexisting;

	  if (fd != -1 && __glibc_unlikely (mode & __RTLD_SECURE)
	      && __libc_enable_secure)
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
	  *realname = (char *) malloc (buflen);
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
      if (here_any && (err = errno) != ENOENT && err != EACCES)
	/* The file exists and is readable, but something went wrong.  */
	return -1;

      /* Remember whether we found anything.  */
      any |= here_any;
    }
  while (*++dirs != NULL);

  /* Remove the whole path if none of the directories exists.  */
  if (__glibc_unlikely (! any))
    {
      /* Paths which were allocated using the minimal malloc() in ld.so
	 must not be freed using the general free() in libc.  */
      if (sps->malloced)
	free (sps->dirs);

      /* rtld_search_dirs and env_path_list are attribute_relro, therefore
	 avoid writing into it.  */
      if (sps != &rtld_search_dirs && sps != &env_path_list)
	sps->dirs = (void *) -1;
    }

  return -1;
}

/* Map in the shared object file NAME.  */

struct link_map *
internal_function
_dl_map_object (struct link_map *loader, const char *name,
		int type, int trace_mode, int mode, Lmid_t nsid)
{
  int fd;
  const char *origname = NULL;
  char *realname;
  char *name_copy;
  struct link_map *l;
  struct filebuf fb;

  assert (nsid >= 0);
  assert (nsid < GL(dl_nns));

  /* Look for this name among those already loaded.  */
  for (l = GL(dl_ns)[nsid]._ns_loaded; l; l = l->l_next)
    {
      /* If the requested name matches the soname of a loaded object,
	 use that object.  Elide this check for names that have not
	 yet been opened.  */
      if (__glibc_unlikely ((l->l_faked | l->l_removed) != 0))
	continue;
      if (!_dl_name_match_p (name, l))
	{
	  const char *soname;

	  if (__glibc_likely (l->l_soname_added)
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
  if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_FILES)
      && loader != NULL)
    _dl_debug_printf ((mode & __RTLD_CALLMAP) == 0
		      ? "\nfile=%s [%lu];  needed by %s [%lu]\n"
		      : "\nfile=%s [%lu];  dynamically loaded by %s [%lu]\n",
		      name, nsid, DSO_FILENAME (loader->l_name), loader->l_ns);

#ifdef SHARED
  /* Give the auditing libraries a chance to change the name before we
     try anything.  */
  if (__glibc_unlikely (GLRO(dl_naudit) > 0)
      && (loader == NULL || loader->l_auditing == 0))
    {
      struct audit_ifaces *afct = GLRO(dl_audit);
      for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
	{
	  if (afct->objsearch != NULL)
	    {
	      const char *before = name;
	      name = afct->objsearch (name, &loader->l_audit[cnt].cookie,
				      LA_SER_ORIG);
	      if (name == NULL)
		{
		  /* Do not try anything further.  */
		  fd = -1;
		  goto no_file;
		}
	      if (before != name && strcmp (before, name) != 0)
		{
		  if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_FILES))
		    _dl_debug_printf ("audit changed filename %s -> %s\n",
				      before, name);

		  if (origname == NULL)
		    origname = before;
		}
	    }

	  afct = afct->next;
	}
    }
#endif

  /* Will be true if we found a DSO which is of the other ELF class.  */
  bool found_other_class = false;

  if (strchr (name, '/') == NULL)
    {
      /* Search for NAME in several places.  */

      size_t namelen = strlen (name) + 1;

      if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_LIBS))
	_dl_debug_printf ("find library=%s [%lu]; searching\n", name, nsid);

      fd = -1;

      /* When the object has the RUNPATH information we don't use any
	 RPATHs.  */
      if (loader == NULL || loader->l_info[DT_RUNPATH] == NULL)
	{
	  /* This is the executable's map (if there is one).  Make sure that
	     we do not look at it twice.  */
	  struct link_map *main_map = GL(dl_ns)[LM_ID_BASE]._ns_loaded;
	  bool did_main_map = false;

	  /* First try the DT_RPATH of the dependent object that caused NAME
	     to be loaded.  Then that object's dependent, and on up.  */
	  for (l = loader; l; l = l->l_loader)
	    if (cache_rpath (l, &l->l_rpath_dirs, DT_RPATH, "RPATH"))
	      {
		fd = open_path (name, namelen, mode,
				&l->l_rpath_dirs,
				&realname, &fb, loader, LA_SER_RUNPATH,
				&found_other_class);
		if (fd != -1)
		  break;

		did_main_map |= l == main_map;
	      }

	  /* If dynamically linked, try the DT_RPATH of the executable
	     itself.  NB: we do this for lookups in any namespace.  */
	  if (fd == -1 && !did_main_map
	      && main_map != NULL && main_map->l_type != lt_loaded
	      && cache_rpath (main_map, &main_map->l_rpath_dirs, DT_RPATH,
			      "RPATH"))
	    fd = open_path (name, namelen, mode,
			    &main_map->l_rpath_dirs,
			    &realname, &fb, loader ?: main_map, LA_SER_RUNPATH,
			    &found_other_class);
	}

      /* Try the LD_LIBRARY_PATH environment variable.  */
      if (fd == -1 && env_path_list.dirs != (void *) -1)
	fd = open_path (name, namelen, mode, &env_path_list,
			&realname, &fb,
			loader ?: GL(dl_ns)[LM_ID_BASE]._ns_loaded,
			LA_SER_LIBPATH, &found_other_class);

      /* Look at the RUNPATH information for this binary.  */
      if (fd == -1 && loader != NULL
	  && cache_rpath (loader, &loader->l_runpath_dirs,
			  DT_RUNPATH, "RUNPATH"))
	fd = open_path (name, namelen, mode,
			&loader->l_runpath_dirs, &realname, &fb, loader,
			LA_SER_RUNPATH, &found_other_class);

      if (fd == -1)
        {
          realname = _dl_sysdep_open_object (name, namelen, &fd);
          if (realname != NULL)
            {
              fd = open_verify (realname, fd,
                                &fb, loader ?: GL(dl_ns)[nsid]._ns_loaded,
                                LA_SER_CONFIG, mode, &found_other_class,
                                false);
              if (fd == -1)
                free (realname);
            }
        }

#ifdef USE_LDCONFIG
      if (fd == -1
	  && (__glibc_likely ((mode & __RTLD_SECURE) == 0)
	      || ! __libc_enable_secure)
	  && __glibc_likely (GLRO(dl_inhibit_cache) == 0))
	{
	  /* Check the list of libraries in the file /etc/ld.so.cache,
	     for compatibility with Linux's ldconfig program.  */
	  char *cached = _dl_load_cache_lookup (name);

	  if (cached != NULL)
	    {
	      // XXX Correct to unconditionally default to namespace 0?
	      l = (loader
		   ?: GL(dl_ns)[LM_ID_BASE]._ns_loaded
# ifdef SHARED
		   ?: &GL(dl_rtld_map)
# endif
		  );

	      /* If the loader has the DF_1_NODEFLIB flag set we must not
		 use a cache entry from any of these directories.  */
	      if (__glibc_unlikely (l->l_flags_1 & DF_1_NODEFLIB))
		{
		  const char *dirp = system_dirs;
		  unsigned int cnt = 0;

		  do
		    {
		      if (memcmp (cached, dirp, system_dirs_len[cnt]) == 0)
			{
			  /* The prefix matches.  Don't use the entry.  */
			  free (cached);
			  cached = NULL;
			  break;
			}

		      dirp += system_dirs_len[cnt] + 1;
		      ++cnt;
		    }
		  while (cnt < nsystem_dirs_len);
		}

	      if (cached != NULL)
		{
		  fd = open_verify (cached, -1,
				    &fb, loader ?: GL(dl_ns)[nsid]._ns_loaded,
				    LA_SER_CONFIG, mode, &found_other_class,
				    false);
		  if (__glibc_likely (fd != -1))
		    realname = cached;
		  else
		    free (cached);
		}
	    }
	}
#endif

      /* Finally, try the default path.  */
      if (fd == -1
	  && ((l = loader ?: GL(dl_ns)[nsid]._ns_loaded) == NULL
	      || __glibc_likely (!(l->l_flags_1 & DF_1_NODEFLIB)))
	  && rtld_search_dirs.dirs != (void *) -1)
	fd = open_path (name, namelen, mode, &rtld_search_dirs,
			&realname, &fb, l, LA_SER_DEFAULT, &found_other_class);

      /* Add another newline when we are tracing the library loading.  */
      if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_LIBS))
	_dl_debug_printf ("\n");
    }
  else
    {
      /* The path may contain dynamic string tokens.  */
      realname = (loader
		  ? expand_dynamic_string_token (loader, name, 0)
		  : __strdup (name));
      if (realname == NULL)
	fd = -1;
      else
	{
	  fd = open_verify (realname, -1, &fb,
			    loader ?: GL(dl_ns)[nsid]._ns_loaded, 0, mode,
			    &found_other_class, true);
	  if (__glibc_unlikely (fd == -1))
	    free (realname);
	}
    }

#ifdef SHARED
 no_file:
#endif
  /* In case the LOADER information has only been provided to get to
     the appropriate RUNPATH/RPATH information we do not need it
     anymore.  */
  if (mode & __RTLD_CALLMAP)
    loader = NULL;

  if (__glibc_unlikely (fd == -1))
    {
      if (trace_mode
	  && __glibc_likely ((GLRO(dl_debug_mask) & DL_DEBUG_PRELINK) == 0))
	{
	  /* We haven't found an appropriate library.  But since we
	     are only interested in the list of libraries this isn't
	     so severe.  Fake an entry with all the information we
	     have.  */
	  static const Elf_Symndx dummy_bucket = STN_UNDEF;

	  /* Allocate a new object map.  */
	  if ((name_copy = __strdup (name)) == NULL
	      || (l = _dl_new_object (name_copy, name, type, loader,
				      mode, nsid)) == NULL)
	    {
	      free (name_copy);
	      _dl_signal_error (ENOMEM, name, NULL,
				N_("cannot create shared object descriptor"));
	    }
	  /* Signal that this is a faked entry.  */
	  l->l_faked = 1;
	  /* Since the descriptor is initialized with zero we do not
	     have do this here.
	  l->l_reserved = 0; */
	  l->l_buckets = &dummy_bucket;
	  l->l_nbuckets = 1;
	  l->l_relocated = 1;

	  /* Enter the object in the object list.  */
	  _dl_add_to_namespace_list (l, nsid);

	  return l;
	}
      else if (found_other_class)
	_dl_signal_error (0, name, NULL,
			  ELFW(CLASS) == ELFCLASS32
			  ? N_("wrong ELF class: ELFCLASS64")
			  : N_("wrong ELF class: ELFCLASS32"));
      else
	_dl_signal_error (errno, name, NULL,
			  N_("cannot open shared object file"));
    }

  void *stack_end = __libc_stack_end;
  return _dl_map_object_from_fd (name, origname, fd, &fb, realname, loader,
				 type, mode, &stack_end, nsid);
}

struct add_path_state
{
  bool counting;
  unsigned int idx;
  Dl_serinfo *si;
  char *allocptr;
};

static void
add_path (struct add_path_state *p, const struct r_search_path_struct *sps,
	  unsigned int flags)
{
  if (sps->dirs != (void *) -1)
    {
      struct r_search_path_elem **dirs = sps->dirs;
      do
	{
	  const struct r_search_path_elem *const r = *dirs++;
	  if (p->counting)
	    {
	      p->si->dls_cnt++;
	      p->si->dls_size += MAX (2, r->dirnamelen);
	    }
	  else
	    {
	      Dl_serpath *const sp = &p->si->dls_serpath[p->idx++];
	      sp->dls_name = p->allocptr;
	      if (r->dirnamelen < 2)
		*p->allocptr++ = r->dirnamelen ? '/' : '.';
	      else
		p->allocptr = __mempcpy (p->allocptr,
					  r->dirname, r->dirnamelen - 1);
	      *p->allocptr++ = '\0';
	      sp->dls_flags = flags;
	    }
	}
      while (*dirs != NULL);
    }
}

void
internal_function
_dl_rtld_di_serinfo (struct link_map *loader, Dl_serinfo *si, bool counting)
{
  if (counting)
    {
      si->dls_cnt = 0;
      si->dls_size = 0;
    }

  struct add_path_state p =
    {
      .counting = counting,
      .idx = 0,
      .si = si,
      .allocptr = (char *) &si->dls_serpath[si->dls_cnt]
    };

# define add_path(p, sps, flags) add_path(p, sps, 0) /* XXX */

  /* When the object has the RUNPATH information we don't use any RPATHs.  */
  if (loader->l_info[DT_RUNPATH] == NULL)
    {
      /* First try the DT_RPATH of the dependent object that caused NAME
	 to be loaded.  Then that object's dependent, and on up.  */

      struct link_map *l = loader;
      do
	{
	  if (cache_rpath (l, &l->l_rpath_dirs, DT_RPATH, "RPATH"))
	    add_path (&p, &l->l_rpath_dirs, XXX_RPATH);
	  l = l->l_loader;
	}
      while (l != NULL);

      /* If dynamically linked, try the DT_RPATH of the executable itself.  */
      if (loader->l_ns == LM_ID_BASE)
	{
	  l = GL(dl_ns)[LM_ID_BASE]._ns_loaded;
	  if (l != NULL && l->l_type != lt_loaded && l != loader)
	    if (cache_rpath (l, &l->l_rpath_dirs, DT_RPATH, "RPATH"))
	      add_path (&p, &l->l_rpath_dirs, XXX_RPATH);
	}
    }

  /* Try the LD_LIBRARY_PATH environment variable.  */
  add_path (&p, &env_path_list, XXX_ENV);

  /* Look at the RUNPATH information for this binary.  */
  if (cache_rpath (loader, &loader->l_runpath_dirs, DT_RUNPATH, "RUNPATH"))
    add_path (&p, &loader->l_runpath_dirs, XXX_RUNPATH);

  /* XXX
     Here is where ld.so.cache gets checked, but we don't have
     a way to indicate that in the results for Dl_serinfo.  */

  /* Finally, try the default path.  */
  if (!(loader->l_flags_1 & DF_1_NODEFLIB))
    add_path (&p, &rtld_search_dirs, XXX_default);

  if (counting)
    /* Count the struct size before the string area, which we didn't
       know before we completed dls_cnt.  */
    si->dls_size += (char *) &si->dls_serpath[si->dls_cnt] - (char *) si;
}
