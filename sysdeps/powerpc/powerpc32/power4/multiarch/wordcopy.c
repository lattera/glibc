/* Multiple versions of wordcopy functions.
   Copyright (C) 2013-2015 Free Software Foundation, Inc.
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

#if IS_IN (libc)
# include <stddef.h>
# include <memcopy.h>
# include <shlib-compat.h>
# include "init-arch.h"

extern __typeof (_wordcopy_fwd_aligned) _wordcopy_fwd_aligned_ppc
attribute_hidden;
extern __typeof (_wordcopy_fwd_aligned) _wordcopy_fwd_aligned_power6
attribute_hidden;
extern __typeof (_wordcopy_fwd_aligned) _wordcopy_fwd_aligned_power7
attribute_hidden;

libc_ifunc (_wordcopy_fwd_aligned,
            (hwcap & PPC_FEATURE_HAS_VSX)
            ? _wordcopy_fwd_aligned_power7 :
	      (hwcap & PPC_FEATURE_ARCH_2_05)
              ? _wordcopy_fwd_aligned_power6
            : _wordcopy_fwd_aligned_ppc);


extern __typeof (_wordcopy_fwd_dest_aligned) _wordcopy_fwd_dest_aligned_ppc
attribute_hidden;
extern __typeof (_wordcopy_fwd_dest_aligned) _wordcopy_fwd_dest_aligned_power6
attribute_hidden;
extern __typeof (_wordcopy_fwd_dest_aligned) _wordcopy_fwd_dest_aligned_power7
attribute_hidden;

libc_ifunc (_wordcopy_fwd_dest_aligned,
            (hwcap & PPC_FEATURE_HAS_VSX)
            ? _wordcopy_fwd_dest_aligned_power7 :
	      (hwcap & PPC_FEATURE_ARCH_2_05)
              ? _wordcopy_fwd_dest_aligned_power6
            : _wordcopy_fwd_dest_aligned_ppc);


extern __typeof (_wordcopy_bwd_aligned) _wordcopy_bwd_aligned_ppc
attribute_hidden;
extern __typeof (_wordcopy_bwd_aligned) _wordcopy_bwd_aligned_power6
attribute_hidden;
extern __typeof (_wordcopy_bwd_aligned) _wordcopy_bwd_aligned_power7
attribute_hidden;

libc_ifunc (_wordcopy_bwd_aligned,
            (hwcap & PPC_FEATURE_HAS_VSX)
            ? _wordcopy_bwd_aligned_power7 :
	      (hwcap & PPC_FEATURE_ARCH_2_05)
              ? _wordcopy_bwd_aligned_power6
            : _wordcopy_bwd_aligned_ppc);


extern __typeof (_wordcopy_bwd_dest_aligned) _wordcopy_bwd_dest_aligned_ppc
attribute_hidden;
extern __typeof (_wordcopy_bwd_dest_aligned) _wordcopy_bwd_dest_aligned_power6
attribute_hidden;
extern __typeof (_wordcopy_bwd_dest_aligned) _wordcopy_bwd_dest_aligned_power7
attribute_hidden;

libc_ifunc (_wordcopy_bwd_dest_aligned,
            (hwcap & PPC_FEATURE_HAS_VSX)
            ? _wordcopy_bwd_dest_aligned_power7 :
	      (hwcap & PPC_FEATURE_ARCH_2_05)
              ? _wordcopy_bwd_dest_aligned_power6
            : _wordcopy_bwd_dest_aligned_ppc);

#else
#include <sysdeps/powerpc/power4/wordcopy.c>
#endif
