/* Map in a shared object's segments.  NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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
#include <dl-load.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <libc-internal.h>


/* This is basically pread, but with iteration after short reads.  */
static bool
read_in_data (int fd, void *data, size_t len, off_t pos)
{
  if (__glibc_unlikely (__lseek (fd, pos, SEEK_SET) == (off_t) -1))
    return true;
  while (len > 0)
    {
      ssize_t n = __read (fd, data, len);
      if (__glibc_unlikely (n < 0))
	return true;
      if (__glibc_unlikely (n == 0))
	{
	  errno = EFTYPE;
	  return true;
	}
      data += n;
      len -= n;
    }
  return false;
}

static const char *
_dl_map_segments (struct link_map *l, int fd,
		  const ElfW(Ehdr) *header, int type,
		  const struct loadcmd loadcmds[], size_t nloadcmds,
		  const size_t maplength, bool has_holes,
		  struct link_map *loader)
{
  if (__glibc_likely (type == ET_DYN))
    {
      /* This is a position-independent shared object.  Let the system
	 choose where to place it.

	 As a refinement, sometimes we have an address that we would
	 prefer to map such objects at; but this is only a preference,
	 the OS can do whatever it likes. */
      ElfW(Addr) mappref
	= (ELF_PREFERRED_ADDRESS (loader, maplength,
				  loadcmds[0].mapstart & GLRO(dl_use_load_bias))
	   - MAP_BASE_ADDR (l));

      uintptr_t mapstart;
      if (__glibc_likely (loadcmds[0].prot & PROT_EXEC))
	{
	  /* When there is a code segment, we must use the
	     allocate_code_data interface to choose a location.  */

	  uintptr_t code_size = loadcmds[0].allocend - loadcmds[0].mapstart;
	  uintptr_t data_offset;
	  size_t data_size;

	  if (__glibc_likely (nloadcmds > 1))
	    {
	      data_offset = loadcmds[1].mapstart - loadcmds[0].mapstart;
	      data_size = ALIGN_UP (maplength - data_offset,
				    GLRO(dl_pagesize));
	    }
	  else
	    {
	      data_offset = 0;
	      data_size = 0;
	    }

	  int error = __nacl_irt_code_data_alloc.allocate_code_data
	    (mappref, code_size, data_offset, data_size, &mapstart);
	  if (__glibc_unlikely (error))
	    {
	      errno = error;
	      return DL_MAP_SEGMENTS_ERROR_MAP_SEGMENT;
	    }
	}
      else
	{
	  /* With no code pages involved, plain mmap works fine.  */
	  void *mapped = __mmap ((void *) mappref, maplength,
				 PROT_NONE, MAP_ANON, -1, 0);
	  if (__glibc_unlikely (mapped == MAP_FAILED))
	    return DL_MAP_SEGMENTS_ERROR_MAP_SEGMENT;
	  mapstart = (uintptr_t) mapped;
	}

      l->l_addr = mapstart - loadcmds[0].mapstart;
    }

  /* Remember which part of the address space this object uses.  */
  l->l_map_start = loadcmds[0].mapstart + l->l_addr;
  l->l_map_end = l->l_map_start + maplength;
  l->l_contiguous = !has_holes;

  /* Now actually map (or read) in each segment.  */
  for (const struct loadcmd *c = loadcmds; c < &loadcmds[nloadcmds]; ++c)
    if (__glibc_likely (c->mapend > c->mapstart))
      {
	/* Unlike POSIX mmap, NaCl's mmap does not reliably handle COW
	   faults in the remainder of the final partial page.  So to get
	   the expected behavior for the unaligned boundary between data
	   and bss, it's necessary to allocate the final partial page of
	   data as anonymous memory rather than mapping it from the file.  */

	size_t maplen = c->mapend - c->mapstart;
	if (c->mapend > c->dataend && c->allocend > c->dataend)
	  maplen = (c->dataend & -GLRO(dl_pagesize)) - c->mapstart;

	/* Map the segment contents from the file.  */
	if (__glibc_unlikely (__mmap ((void *) (l->l_addr + c->mapstart),
				      maplen, c->prot,
				      MAP_FIXED|MAP_COPY|MAP_FILE,
				      fd, c->mapoff)
			      == MAP_FAILED))
	  {
	    switch (errno)
	      {
	      case EINVAL:
	      case ENOTSUP:
	      case ENOSYS:
		break;
	      default:
		return DL_MAP_SEGMENTS_ERROR_MAP_SEGMENT;
	      }

	    /* No mmap support for this file.  */
	    if (c->prot & PROT_EXEC)
	      {
		/* Read the data into a temporary buffer.  */
		const size_t len = c->mapend - c->mapstart;
		void *data = __mmap (NULL, len, PROT_READ | PROT_WRITE,
				     MAP_ANON|MAP_PRIVATE, -1, 0);
		if (__glibc_unlikely (data == MAP_FAILED))
		  return DL_MAP_SEGMENTS_ERROR_MAP_ZERO_FILL;
		if (read_in_data (fd, data, len, c->mapoff))
		  return DL_MAP_SEGMENTS_ERROR_MAP_SEGMENT;
		/* Now validate and install the code.  */
		int error = __nacl_irt_dyncode.dyncode_create
		  ((void *) (l->l_addr + c->mapstart), data, len);
		__munmap (data, len);
		if (__glibc_unlikely (error))
		  {
		    errno = error;
		    return DL_MAP_SEGMENTS_ERROR_MAP_SEGMENT;
		  }
                if (__glibc_unlikely (type != ET_DYN))
                  {
                    /* A successful PROT_EXEC mmap would have implicitly
                       updated the bookkeeping so that a future
                       allocate_code_data call would know that this range
                       of the address space is already occupied.  That
                       doesn't happen implicitly with dyncode_create, so
                       it's necessary to do an explicit call to update the
                       bookkeeping.  */
                    uintptr_t allocated_address;
                    error = __nacl_irt_code_data_alloc.allocate_code_data
                      (l->l_addr + c->mapstart, len, 0, 0, &allocated_address);
                    if (__glibc_unlikely (error))
                      {
                        errno = error;
                        return DL_MAP_SEGMENTS_ERROR_MAP_SEGMENT;
                      }
                    if (__glibc_unlikely
                        (allocated_address != l->l_addr + c->mapstart))
                      {
                        /* This is not a very helpful error for this case,
                           but there isn't really anything better to use.  */
                        errno = ENOMEM;
                        return DL_MAP_SEGMENTS_ERROR_MAP_SEGMENT;
                      }
                  }
	      }
	    else
	      {
		/* Allocate the pages.  */
		if (__mmap ((void *) (l->l_addr + c->mapstart),
			    c->mapend - c->mapstart, c->prot | PROT_WRITE,
			    MAP_FIXED|MAP_ANON|MAP_PRIVATE, -1, 0)
		    == MAP_FAILED)
		  return DL_MAP_SEGMENTS_ERROR_MAP_ZERO_FILL;
		/* Now read in the data.  */
		if (read_in_data (fd, (void *) (l->l_addr + c->mapstart),
				  c->dataend - c->mapstart, c->mapoff))
		  return DL_MAP_SEGMENTS_ERROR_MAP_SEGMENT;
		/* Now that we've filled the pages, reset the page
		   protections to what they should be.  */
		if (!(c->prot & PROT_WRITE)
		    && __mprotect ((void *) (l->l_addr + c->mapstart),
				   c->mapend - c->mapstart, c->prot) < 0)
		  return DL_MAP_SEGMENTS_ERROR_MPROTECT;
	      }
	  }
	else if (c->allocend > c->dataend)
	  {
	    /* Extra zero pages should appear at the end of this segment,
	       after the data mapped from the file.   */

	    uintptr_t allocend = c->mapend;
	    if (c->mapend > c->dataend)
	      {
		/* The final data page was partial.  So we didn't map it in.
		   Instead, we must allocate an anonymous page to fill.  */
		if (c->prot & PROT_WRITE)
		  /* Do the whole allocation right here.  */
		  allocend = c->allocend;
		if (__mmap ((void *) (l->l_addr + c->mapstart + maplen),
			    allocend - (c->mapstart + maplen), c->prot,
			    MAP_FIXED|MAP_ANON|MAP_PRIVATE, -1, 0)
		    == MAP_FAILED)
		  return DL_MAP_SEGMENTS_ERROR_MAP_ZERO_FILL;
		if (read_in_data (fd,
				  (void *) (l->l_addr + c->mapstart + maplen),
				  c->dataend & (GLRO(dl_pagesize) - 1),
				  c->mapoff + maplen))
		  return DL_MAP_SEGMENTS_ERROR_MAP_SEGMENT;
		/* Now that we've filled the page, reset its
		   protections to what they should be.  */
		if (!(c->prot & PROT_WRITE)
		    && __mprotect ((void *) (l->l_addr + c->mapstart + maplen),
				   c->mapend - (c->mapstart + maplen),
				   c->prot) < 0)
		  return DL_MAP_SEGMENTS_ERROR_MPROTECT;
	      }

	    /* Now allocate the pure zero-fill pages.  */
	    if (allocend < c->allocend
		&& (__mmap ((void *) (l->l_addr + c->mapstart + allocend),
			   c->allocend - (c->mapstart + allocend), c->prot,
			   MAP_FIXED|MAP_ANON|MAP_PRIVATE, -1, 0)
		    == MAP_FAILED))
	      return DL_MAP_SEGMENTS_ERROR_MAP_ZERO_FILL;
	  }

	_dl_postprocess_loadcmd (l, header, c);
      }

  /* Notify ELF_PREFERRED_ADDRESS that we have to load this one
     fixed.  */
  ELF_FIXED_ADDRESS (loader, c->mapstart);

  return NULL;
}
