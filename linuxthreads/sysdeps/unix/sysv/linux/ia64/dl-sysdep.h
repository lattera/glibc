/* System-specific settings for dynamic linker code.  IA-64 version.
   Copyright (C) 2003, 2004 Free Software Foundation, Inc.
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

#ifndef _DL_SYSDEP_H
#define _DL_SYSDEP_H	1

#define NEED_DL_SYSINFO	1
#undef USE_DL_SYSINFO

#if defined NEED_DL_SYSINFO && !defined __ASSEMBLER__
/* Don't declare this as a function---we want it's entry-point, not
   it's function descriptor... */
extern int _dl_sysinfo_break attribute_hidden;
# define DL_SYSINFO_DEFAULT ((uintptr_t) &_dl_sysinfo_break)
# define DL_SYSINFO_IMPLEMENTATION		\
  asm (".text\n\t"				\
       ".hidden _dl_sysinfo_break\n\t"		\
       ".proc _dl_sysinfo_break\n\t"		\
       "_dl_sysinfo_break:\n\t"			\
       ".prologue\n\t"				\
       ".altrp b6\n\t"				\
       ".body\n\t"				\
       "break 0x100000;\n\t"			\
       "br.ret.sptk.many b6;\n\t"		\
       ".endp _dl_sysinfo_break"		\
       ".previous");
#endif

/* _dl_argv cannot be attribute_relro, because _dl_start_user
   might write into it after _dl_start returns.  */
#define DL_ARGV_NOT_RELRO 1

#endif	/* dl-sysdep.h */
