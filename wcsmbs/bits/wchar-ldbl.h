/* -mlong-double-64 compatibility mode for <wchar.h> functions.
   Copyright (C) 2006 Free Software Foundation, Inc.
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

#ifndef _WCHAR_H
# error "Never include <bits/wchar-ldbl.h> directly; use <wchar.h> instead."
#endif

#if defined __LDBL_COMPAT && defined __GNUC__ && __GNUC__ >= 2

# define __LDBL_REDIR_WCHAR(name) \
  extern __typeof (name) name __asm (__ASMNAME (__nldbl_##name))

# if defined __USE_ISOC99 || defined __USE_UNIX98
__BEGIN_NAMESPACE_C99
__LDBL_REDIR_WCHAR (fwprintf);
__LDBL_REDIR_WCHAR (wprintf);
__LDBL_REDIR_WCHAR (swprintf);
__LDBL_REDIR_WCHAR (vfwprintf);
__LDBL_REDIR_WCHAR (vwprintf);
__LDBL_REDIR_WCHAR (vswprintf);
__LDBL_REDIR_WCHAR (fwscanf);
__LDBL_REDIR_WCHAR (wscanf);
__LDBL_REDIR_WCHAR (swscanf);
__END_NAMESPACE_C99
# endif

# ifdef __USE_ISOC99
__BEGIN_NAMESPACE_C99
__LDBL_REDIR_WCHAR (vfwscanf);
__LDBL_REDIR_WCHAR (vwscanf);
__LDBL_REDIR_WCHAR (vswscanf);
__END_NAMESPACE_C99
# endif

#if __USE_FORTIFY_LEVEL > 0 && !defined __cplusplus
__LDBL_REDIR_DECL (__swprintf_chk)
__LDBL_REDIR_DECL (__vswprintf_chk)
# if __USE_FORTIFY_LEVEL > 1
__LDBL_REDIR_DECL (__fwprintf_chk)
__LDBL_REDIR_DECL (__wprintf_chk)
__LDBL_REDIR_DECL (__vfwprintf_chk)
__LDBL_REDIR_DECL (__vwprintf_chk)
# endif
#endif
