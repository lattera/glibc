/* Map addresses from a file or anonymous memory.  NaCl version.
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

#include <errno.h>
#include <sys/mman.h>
#include <nacl-interfaces.h>


/* Map addresses starting near ADDR and extending for LEN bytes.  From
   OFFSET into the file FD describes according to PROT and FLAGS.  If ADDR
   is nonzero, it is the desired mapping address.  If the MAP_FIXED bit is
   set in FLAGS, the mapping will be at ADDR exactly (which must be
   page-aligned); otherwise the system chooses a convenient nearby address.
   The return value is the actual mapping address chosen or MAP_FAILED
   for errors (in which case `errno' is set).  A successful `mmap' call
   deallocates any previous mapping for the affected region.  */

__ptr_t
__mmap (__ptr_t addr, size_t len, int prot, int flags, int fd, off_t offset)
{
  int error = __nacl_irt_memory.mmap (&addr, len, prot, flags, fd, offset);
  if (error)
    {
      errno = error;
      return MAP_FAILED;
    }
  return addr;
}
weak_alias (__mmap, mmap)


/* Since off64_t is the same as off_t, mmap64 is just an alias.  */
strong_alias (__mmap, __mmap64)
weak_alias (__mmap, mmap64)
