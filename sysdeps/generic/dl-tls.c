/* Thread-local storage handling in the ELF dynamic linker.  Generic version.
   Copyright (C) 2002 Free Software Foundation, Inc.
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

#include <assert.h>
#include <stdlib.h>

#include <tls.h>

/* We don't need any of this if TLS is not supported.  */
#ifdef USE_TLS

#include <dl-tls.h>
#include <ldsodefs.h>

/* Value used for dtv entries for which the allocation is delayed.  */
# define TLS_DTV_UNALLOCATE	((void *) -1l)


size_t
internal_function
_dl_next_tls_modid (void)
{
  size_t result;

  if (__builtin_expect (GL(dl_tls_dtv_gaps), false))
    {
      /* XXX If this method proves too costly we can optimize
	 it to use a constant time method.  But I don't think
	 it's a problem.  */
      struct link_map *runp = GL(dl_initimage_list);
      bool used[GL(dl_tls_max_dtv_idx)];

      assert (runp != NULL);
      do
	{
	  assert (runp->l_tls_modid > 0
		  && runp->l_tls_modid <= GL(dl_tls_max_dtv_idx));
	  used[runp->l_tls_modid - 1] = true;
	}
      while ((runp = runp->l_tls_nextimage) != GL(dl_initimage_list));

      result = 0;
      do
	/* The information about the gaps is pessimistic.  It might be
	   there are actually none.  */
	if (result >= GL(dl_tls_max_dtv_idx))
	  {
	    /* Now we know there is actually no gap.  Bump the maximum
	       ID number and remember that there are no gaps.  */
	    result = ++GL(dl_tls_max_dtv_idx);
	    GL(dl_tls_dtv_gaps) = false;
	    break;
	  }
      while (used[result++]);
    }
  else
    /* No gaps, allocate a new entry.  */
    result = ++GL(dl_tls_max_dtv_idx);

  return result;
}


void
internal_function
_dl_determine_tlsoffset (struct link_map *firstp)
{
  struct link_map *runp = firstp;
  size_t max_align = 0;
  size_t offset;

  if (GL(dl_initimage_list) == NULL)
    {
      /* None of the objects used at startup time uses TLS.  We still
	 have to allocate the TCB adn dtv.  */
      GL(dl_tls_static_size) = TLS_TCB_SIZE;
      GL(dl_tls_static_align) = TLS_TCB_ALIGN;

      return;
    }

# if TLS_TCB_AT_TP
  /* We simply start with zero.  */
  offset = 0;

  do
    {
      max_align = MAX (max_align, runp->l_tls_align);

      /* Compute the offset of the next TLS block.  */
      offset = roundup (offset + runp->l_tls_blocksize, runp->l_tls_align);

      /* XXX For some architectures we perhaps should store the
	 negative offset.  */
      runp->l_tls_offset = offset;
    }
  while ((runp = runp->l_tls_nextimage) != firstp);

#if 0
  /* The thread descriptor (pointed to by the thread pointer) has its
     own alignment requirement.  Adjust the static TLS size
     and TLS offsets appropriately.  */
  // XXX How to deal with this.  We cannot simply add zero bytes
  // XXX after the first (closest to the TCB) TLS block since this
  // XXX would invalidate the offsets the linker creates for the LE
  // XXX model.
  if (offset % TLS_TCB_ALIGN != 0)
    abort ();
#endif

  GL(dl_tls_static_size) = offset + TLS_TCB_SIZE;
# elif TLS_DTV_AT_TP
  struct link_map *lastp;

  /* The first block starts right after the TCB.  */
  offset = TLS_TCB_SIZE;
  max_align = runp->l_tls_align;
  runp->l_tls_offset = offset;
  lastp = runp;

  while ((runp = runp->l_tls_nextimage) != firstp)
    {
      max_align = MAX (max_align, runp->l_tls_align);

      /* Compute the offset of the next TLS block.  */
      offset = roundup (offset + lastp->l_tls_blocksize, runp->l_tls_align);

      runp->l_tls_offset = offset;

      lastp = runp;
    }

  GL(dl_tls_static_size) = offset + lastp->l_tls_blocksize;
# else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
# endif

  /* The alignment requirement for the static TLS block.  */
  GL(dl_tls_static_align) = MAX (TLS_TCB_ALIGN, max_align);
}


void *
internal_function
_dl_allocate_tls (void)
{
  void *result;
  dtv_t *dtv;

  /* Allocate a correctly aligned chunk of memory.  */
  /* XXX For now */
  assert (GL(dl_tls_static_align) <= GL(dl_pagesize));
#ifdef MAP_ANON
# define _dl_zerofd (-1)
#else
# define _dl_zerofd GL(dl_zerofd)
  if ((dl_zerofd) == -1)
    GL(dl_zerofd) = _dl_sysdep_open_zero_fill ();
# define MAP_ANON 0
#endif
  result = __mmap (0, GL(dl_tls_static_size), PROT_READ|PROT_WRITE,
		   MAP_ANON|MAP_PRIVATE, _dl_zerofd, 0);

  dtv = (dtv_t *) malloc ((GL(dl_tls_max_dtv_idx) + 1) * sizeof (dtv_t));
  if (result != MAP_FAILED && dtv != NULL)
    {
      struct link_map *runp;

# if TLS_TCB_AT_TP
      /* The TCB follows the TLS blocks.  */
      result = (char *) result + GL(dl_tls_static_size) - TLS_TCB_SIZE;
# endif

      /* XXX Fill in an correct generation number.  */
      dtv[0].counter = 0;

      /* Initialize the memory from the initialization image list and clear
	 the BSS parts.  */
      if (GL(dl_initimage_list) != NULL)
	{
	  runp = GL(dl_initimage_list)->l_tls_nextimage;
	  do
	    {
	      assert (runp->l_tls_modid > 0);
	      assert (runp->l_tls_modid <= GL(dl_tls_max_dtv_idx));
# if TLS_TCB_AT_TP
	      dtv[runp->l_tls_modid].pointer = result - runp->l_tls_offset;
# elif TLS_DTV_AT_TP
	      dtv[runp->l_tls_modid].pointer = result + runp->l_tls_offset;
# else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
# endif

	      memset (__mempcpy (dtv[runp->l_tls_modid].pointer,
				 runp->l_tls_initimage,
				 runp->l_tls_initimage_size),
		      '\0',
		      runp->l_tls_blocksize - runp->l_tls_initimage_size);
	    }
	  while ((runp = runp->l_tls_nextimage)
		 !=  GL(dl_initimage_list)->l_tls_nextimage);
	}

      /* Add the dtv to the thread data structures.  */
      INSTALL_DTV (result, dtv);
    }
  else if (result != NULL)
    {
      free (result);
      result = NULL;
    }

  return result;
}


/* The __tls_get_addr function has two basic forms which differ in the
   arguments.  The IA-64 form takes two parameters, the module ID and
   offset.  The form used, among others, on IA-32 takes a reference to
   a special structure which contain the same information.  The second
   form seems to be more often used (in the moment) so we default to
   it.  Users of the IA-64 form have to provide adequate definitions
   of the following macros.  */
# ifndef GET_ADDR_ARGS
#  define GET_ADDR_ARGS tls_index *ti
# endif
# ifndef GET_ADDR_MODULE
#  define GET_ADDR_MODULE ti->ti_module
# endif
# ifndef GET_ADDR_OFFSET
#  define GET_ADDR_OFFSET ti->ti_offset
# endif


void *
__tls_get_addr (GET_ADDR_ARGS)
{
  dtv_t *dtv = THREAD_DTV ();

  if (dtv[GET_ADDR_MODULE].pointer == TLS_DTV_UNALLOCATE)
    /* XXX */;

  return (char *) dtv[GET_ADDR_MODULE].pointer + GET_ADDR_OFFSET;
}

#endif	/* use TLS */
