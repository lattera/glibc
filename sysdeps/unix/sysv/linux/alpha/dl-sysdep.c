/* Operating system support for run-time dynamic linker.  Linux/PPC version.
   Copyright (C) 1997, 1998, 2001, 2003 Free Software Foundation, Inc.
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


#include "config.h"
#include "kernel-features.h"
#include <ldsodefs.h>

extern long __libc_alpha_cache_shape[4];
weak_extern (__libc_alpha_cache_shape);


/* Scan the Aux Vector for the cache shape entries.  */
#define DL_PLATFORM_AUXV				\
      case AT_L1I_CACHESHAPE:				\
	{						\
	  long *cls = __libc_alpha_cache_shape;		\
	  if (cls != NULL)				\
	    cls[0] = av->a_un.a_val;			\
	  break;					\
	}						\
      case AT_L1D_CACHESHAPE:				\
	{						\
	  long *cls = __libc_alpha_cache_shape;		\
	  if (cls != NULL)				\
	    cls[1] = av->a_un.a_val;			\
	  break;					\
	}						\
      case AT_L2_CACHESHAPE:				\
	{						\
	  long *cls = __libc_alpha_cache_shape;		\
	  if (cls != NULL)				\
	    cls[2] = av->a_un.a_val;			\
	  break;					\
	}						\
      case AT_L3_CACHESHAPE:				\
	{						\
	  long *cls = __libc_alpha_cache_shape;		\
	  if (cls != NULL)				\
	    cls[3] = av->a_un.a_val;			\
	  break;					\
	}

#include <sysdeps/unix/sysv/linux/dl-sysdep.c>
