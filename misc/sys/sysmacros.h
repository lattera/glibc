/* Definitions of macros to access `dev_t' values.
   Copyright (C) 1996-2015 Free Software Foundation, Inc.
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

#ifndef _SYS_SYSMACROS_H_OUTER

#ifndef __SYSMACROS_DEPRECATED_INCLUSION
# define _SYS_SYSMACROS_H_OUTER 1
#endif

/* If <sys/sysmacros.h> is included after <sys/types.h>, these macros
   will already be defined, and we need to redefine them without the
   deprecation warnings.  (If they are included in the opposite order,
   the outer #ifndef will suppress this entire file and the macros
   will be usable without warnings.)  */
#undef major
#undef minor
#undef makedev

/* This is the macro that must be defined to satisfy the misuse check
   in bits/sysmacros.h. */
#ifndef _SYS_SYSMACROS_H
#define _SYS_SYSMACROS_H 1

#include <features.h>
#include <bits/types.h>
#include <bits/sysmacros.h>

/* The extra "\n " moves gcc's [-Wdeprecated-declarations] annotation
   onto the next line.  */
#define __SYSMACROS_DEPRECATION_MSG(symbol)				     \
  "\n  In the GNU C Library, `" #symbol "' is defined by <sys/sysmacros.h>." \
  "\n  For historical compatibility, it is currently defined by"	     \
  "\n  <sys/types.h> as well, but we plan to remove this soon."		     \
  "\n  To use `" #symbol "', include <sys/sysmacros.h> directly."	     \
  "\n  If you did not intend to use a system-defined macro `" #symbol "',"   \
  "\n  you should #undef it after including <sys/types.h>."		     \
  "\n "

#define __SYSMACROS_DECL_TEMPL(rtype, name, proto)			     \
  extern rtype gnu_dev_##name proto __THROW __attribute_const__;

#define __SYSMACROS_FST_DECL_TEMPL(rtype, name, proto)			     \
  extern rtype __REDIRECT_NTH (__##name##_from_sys_types, proto,	     \
			       gnu_dev_##name)				     \
       __attribute_const__						     \
       __attribute_deprecated_msg__ (__SYSMACROS_DEPRECATION_MSG (name));

#define __SYSMACROS_IMPL_TEMPL(rtype, name, proto)			     \
  __extension__ __extern_inline __attribute_const__ rtype		     \
  __NTH (gnu_dev_##name proto)

#define __SYSMACROS_FST_IMPL_TEMPL(rtype, name, proto)			     \
  __extension__ __extern_inline __attribute_const__ rtype		     \
  __NTH (__##name##_from_sys_types proto)

__BEGIN_DECLS

__SYSMACROS_DECLARE_MAJOR (__SYSMACROS_DECL_TEMPL)
__SYSMACROS_DECLARE_MINOR (__SYSMACROS_DECL_TEMPL)
__SYSMACROS_DECLARE_MAKEDEV (__SYSMACROS_DECL_TEMPL)

__SYSMACROS_DECLARE_MAJOR (__SYSMACROS_FST_DECL_TEMPL)
__SYSMACROS_DECLARE_MINOR (__SYSMACROS_FST_DECL_TEMPL)
__SYSMACROS_DECLARE_MAKEDEV (__SYSMACROS_FST_DECL_TEMPL)

#ifdef __USE_EXTERN_INLINES

__SYSMACROS_DEFINE_MAJOR (__SYSMACROS_IMPL_TEMPL)
__SYSMACROS_DEFINE_MINOR (__SYSMACROS_IMPL_TEMPL)
__SYSMACROS_DEFINE_MAKEDEV (__SYSMACROS_IMPL_TEMPL)

__SYSMACROS_DEFINE_MAJOR (__SYSMACROS_FST_IMPL_TEMPL)
__SYSMACROS_DEFINE_MINOR (__SYSMACROS_FST_IMPL_TEMPL)
__SYSMACROS_DEFINE_MAKEDEV (__SYSMACROS_FST_IMPL_TEMPL)

#endif

__END_DECLS

#endif /* _SYS_SYSMACROS_H */

#ifndef __SYSMACROS_NEED_IMPLEMENTATION
# undef __SYSMACROS_DECL_TEMPL
# undef __SYSMACROS_FST_DECL_TEMPL
# undef __SYSMACROS_IMPL_TEMPL
# undef __SYSMACROS_FST_IMPL_TEMPL
# undef __SYSMACROS_DECLARE_MAJOR
# undef __SYSMACROS_DECLARE_MINOR
# undef __SYSMACROS_DECLARE_MAKEDEV
# undef __SYSMACROS_DEFINE_MAJOR
# undef __SYSMACROS_DEFINE_MINOR
# undef __SYSMACROS_DEFINE_MAKEDEV
#endif

#ifdef __SYSMACROS_DEPRECATED_INCLUSION
# define major(dev) __major_from_sys_types (dev)
# define minor(dev) __minor_from_sys_types (dev)
# define makedev(maj, min) __makedev_from_sys_types (maj, min)
#else
# define major(dev) gnu_dev_major (dev)
# define minor(dev) gnu_dev_minor (dev)
# define makedev(maj, min) gnu_dev_makedev (maj, min)
#endif

#endif /* sys/sysmacros.h */
