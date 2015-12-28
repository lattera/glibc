/* Data for processor capability information.  PowerPC version.
   Copyright (C) 2005-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* This information must be kept in sync with the _DL_HWCAP_COUNT and
   _DL_PLATFORM_COUNT definitions in procinfo.h.

   If anything should be added here check whether the size of each string
   is still ok with the given array size.

   All the #ifdefs in the definitions are quite irritating but
   necessary if we want to avoid duplicating the information.  There
   are three different modes:

   - PROCINFO_DECL is defined.  This means we are only interested in
     declarations.

   - PROCINFO_DECL is not defined:

     + if SHARED is defined the file is included in an array
       initializer.  The .element = { ... } syntax is needed.

     + if SHARED is not defined a normal array initialization is
       needed.
  */

#ifndef PROCINFO_CLASS
# define PROCINFO_CLASS
#endif

#if !defined PROCINFO_DECL && defined SHARED
  ._dl_powerpc_cap_flags
#else
PROCINFO_CLASS const char _dl_powerpc_cap_flags[64][10]
#endif
#ifndef PROCINFO_DECL
= {
    "ppcle", "true_le", "", "",
    "", "", "archpmu", "vsx",
    "arch_2_06", "power6x", "dfp", "pa6t",
    "arch_2_05", "ic_snoop", "smt", "booke",
    "cellbe", "power5+", "power5", "power4",
    "notb", "efpdouble", "efpsingle", "spe",
    "ucache", "4xxmac", "mmu", "fpu",
    "altivec", "ppc601", "ppc64", "ppc32",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "ieee128", "arch_3_00",
    "htm-nosc", "vcrypto", "tar", "isel",
    "ebb", "dscr", "htm", "arch_2_07",
  }
#endif
#if !defined SHARED || defined PROCINFO_DECL
;
#else
,
#endif

#if !defined PROCINFO_DECL && defined SHARED
  ._dl_powerpc_platforms
#else
PROCINFO_CLASS const char _dl_powerpc_platforms[15][12]
#endif
#ifndef PROCINFO_DECL
= {
    [PPC_PLATFORM_POWER4] = "power4",
    [PPC_PLATFORM_PPC970] = "ppc970",
    [PPC_PLATFORM_POWER5] = "power5",
    [PPC_PLATFORM_POWER5_PLUS] = "power5+",
    [PPC_PLATFORM_POWER6] = "power6",
    [PPC_PLATFORM_CELL_BE] = "ppc-cell-be",
    [PPC_PLATFORM_POWER6X] = "power6x",
    [PPC_PLATFORM_POWER7] = "power7",
    [PPC_PLATFORM_PPCA2] = "ppca2",
    [PPC_PLATFORM_PPC405] = "ppc405",
    [PPC_PLATFORM_PPC440] = "ppc440",
    [PPC_PLATFORM_PPC464] = "ppc464",
    [PPC_PLATFORM_PPC476] = "ppc476",
    [PPC_PLATFORM_POWER8] = "power8",
    [PPC_PLATFORM_POWER9] = "power9"
  }
#endif
#if !defined SHARED || defined PROCINFO_DECL
;
#else
,
#endif

#undef PROCINFO_DECL
#undef PROCINFO_CLASS
