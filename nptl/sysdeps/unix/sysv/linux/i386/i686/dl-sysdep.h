/* System-specific settings for dynamic linker code.  IA-32 version.
   Copyright (C) 2002-2012 Free Software Foundation, Inc.
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

#ifndef _DL_SYSDEP_H
#define _DL_SYSDEP_H	1

/* This macro must be defined to either 0 or 1.

   If 1, then an errno global variable hidden in ld.so will work right with
   all the errno-using libc code compiled for ld.so, and there is never a
   need to share the errno location with libc.  This is appropriate only if
   all the libc functions that ld.so uses are called without PLT and always
   get the versions linked into ld.so rather than the libc ones.  */

#ifdef IS_IN_rtld
# define RTLD_PRIVATE_ERRNO 1
#else
# define RTLD_PRIVATE_ERRNO 0
#endif

/* Traditionally system calls have been made using int $0x80.  A
   second method was introduced which, if possible, will use the
   sysenter/syscall instructions.  To signal the presence and where to
   find the code the kernel passes an AT_SYSINFO value in the
   auxiliary vector to the application.  */
#define NEED_DL_SYSINFO	1
#define USE_DL_SYSINFO	1

#if defined NEED_DL_SYSINFO && !defined __ASSEMBLER__
extern void _dl_sysinfo_int80 (void) attribute_hidden;
# define DL_SYSINFO_DEFAULT (uintptr_t) _dl_sysinfo_int80
# define DL_SYSINFO_IMPLEMENTATION \
  asm (".text\n\t"							      \
       ".type _dl_sysinfo_int80,@function\n\t"				      \
       ".hidden _dl_sysinfo_int80\n"					      \
       CFI_STARTPROC "\n"						      \
       "_dl_sysinfo_int80:\n\t"						      \
       "int $0x80;\n\t"							      \
       "ret;\n\t"							      \
       CFI_ENDPROC "\n"							      \
       ".size _dl_sysinfo_int80,.-_dl_sysinfo_int80\n\t"		      \
       ".previous");
#endif


#ifndef __ASSEMBLER__
/* Get version of the OS.  */
extern int _dl_discover_osversion (void) attribute_hidden;
# define HAVE_DL_DISCOVER_OSVERSION	1
#endif

#endif	/* dl-sysdep.h */
