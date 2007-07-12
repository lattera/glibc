/* Provide gconv stub functions for the minimum static binaries.
   Copyright (C) 1999, 2001, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <features.h>
#include <string.h>
#include <wchar.h>
#include <bits/libc-lock.h>
#if __GNUC_PREREQ(3, 3)
# include <gconv_int.h>
#else
# include <gconv.h>
#endif

__libc_lock_define_initialized (, __gconv_lock)

/* hack for self identification */
int __c_stubs_is_compiled_in;

/* Don't drag in the dynamic linker. */
void *__libc_stack_end;

int attribute_hidden
__gconv_OK (void)
{
#if __GLIBC__ > 2 || __GLIBC_MINOR__ > 1
  return __GCONV_OK;
#else
  return GCONV_OK;
#endif
}

int attribute_hidden
__gconv_NOCONV (void)
{
#if __GLIBC__ > 2 || __GLIBC_MINOR__ > 1
  return __GCONV_NOCONV;
#else
  return GCONV_NOCONV;
#endif
}

void attribute_hidden
__gconv_NOOP (void)
{
}

int
internal_function
__gconv_compare_alias (const char *name1, const char *name2)
{
  return strcmp (name1, name2);
}

wint_t
__gconv_btwoc_ascii (struct __gconv_step *step, unsigned char c)
{
  if (c < 0x80)
    return c;
  else
    return WEOF;
}


#if __GNUC_PREREQ(3, 3)
# undef strong_alias
# define strong_alias(impl, name) \
  __typeof (name) name __attribute__ ((alias (#impl)))
#endif

strong_alias (__gconv_OK,
	      __gconv_close_transform);
strong_alias (__gconv_OK,
	      __gconv_close);

strong_alias (__gconv_NOCONV,
	      __gconv);
strong_alias (__gconv_NOCONV,
	      __gconv_find_transform);
strong_alias (__gconv_NOCONV,
	      __gconv_open);

/* These transformations should not fail in normal conditions */
strong_alias (__gconv_OK,
	      __gconv_transform_ascii_internal);
strong_alias (__gconv_OK,
	      __gconv_transform_utf16_internal);
strong_alias (__gconv_OK,
	      __gconv_transform_utf8_internal);
strong_alias (__gconv_OK,
	      __gconv_transform_ucs2_internal);

/* We can assume no conversion for these ones */
strong_alias (__gconv_NOCONV,
	      __gconv_transform_internal_ascii);
strong_alias (__gconv_NOCONV,
	      __gconv_transform_internal_ucs2);
strong_alias (__gconv_NOCONV,
	      __gconv_transform_internal_ucs4);
strong_alias (__gconv_NOCONV,
	      __gconv_transform_internal_utf16);
strong_alias (__gconv_NOCONV,
	      __gconv_transform_internal_utf8);

strong_alias (__gconv_NOCONV,
	      __gconv_transliterate);

strong_alias (__gconv_NOOP,
	      __gconv_release_cache);
strong_alias (__gconv_NOOP,
	      __gconv_release_step);
