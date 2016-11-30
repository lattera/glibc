/* Minimal replacements for basic facilities used in the dynamic linker.
   Copyright (C) 1995-2016 Free Software Foundation, Inc.
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
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <tls.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/types.h>
#include <ldsodefs.h>
#include <_itoa.h>
#include <malloc/malloc-internal.h>

#include <assert.h>

/* Minimal malloc allocator for used during initial link.  After the
   initial link, a full malloc implementation is interposed, either
   the one in libc, or a different one supplied by the user through
   interposition.  */

static void *alloc_ptr, *alloc_end, *alloc_last_block;

/* Declarations of global functions.  */
extern void weak_function free (void *ptr);
extern void * weak_function realloc (void *ptr, size_t n);
extern unsigned long int weak_function __strtoul_internal (const char *nptr,
							   char **endptr,
							   int base,
							   int group);
extern unsigned long int weak_function strtoul (const char *nptr,
						char **endptr, int base);


/* Allocate an aligned memory block.  */
void * weak_function
malloc (size_t n)
{
  if (alloc_end == 0)
    {
      /* Consume any unused space in the last page of our data segment.  */
      extern int _end attribute_hidden;
      alloc_ptr = &_end;
      alloc_end = (void *) 0 + (((alloc_ptr - (void *) 0)
				 + GLRO(dl_pagesize) - 1)
				& ~(GLRO(dl_pagesize) - 1));
    }

  /* Make sure the allocation pointer is ideally aligned.  */
  alloc_ptr = (void *) 0 + (((alloc_ptr - (void *) 0) + MALLOC_ALIGNMENT - 1)
			    & ~(MALLOC_ALIGNMENT - 1));

  if (alloc_ptr + n >= alloc_end || n >= -(uintptr_t) alloc_ptr)
    {
      /* Insufficient space left; allocate another page plus one extra
	 page to reduce number of mmap calls.  */
      caddr_t page;
      size_t nup = (n + GLRO(dl_pagesize) - 1) & ~(GLRO(dl_pagesize) - 1);
      if (__glibc_unlikely (nup == 0 && n != 0))
	return NULL;
      nup += GLRO(dl_pagesize);
      page = __mmap (0, nup, PROT_READ|PROT_WRITE,
		     MAP_ANON|MAP_PRIVATE, -1, 0);
      if (page == MAP_FAILED)
	return NULL;
      if (page != alloc_end)
	alloc_ptr = page;
      alloc_end = page + nup;
    }

  alloc_last_block = (void *) alloc_ptr;
  alloc_ptr += n;
  return alloc_last_block;
}

/* We use this function occasionally since the real implementation may
   be optimized when it can assume the memory it returns already is
   set to NUL.  */
void * weak_function
calloc (size_t nmemb, size_t size)
{
  /* New memory from the trivial malloc above is always already cleared.
     (We make sure that's true in the rare occasion it might not be,
     by clearing memory in free, below.)  */
  size_t bytes = nmemb * size;

#define HALF_SIZE_T (((size_t) 1) << (8 * sizeof (size_t) / 2))
  if (__builtin_expect ((nmemb | size) >= HALF_SIZE_T, 0)
      && size != 0 && bytes / size != nmemb)
    return NULL;

  return malloc (bytes);
}

/* This will rarely be called.  */
void weak_function
free (void *ptr)
{
  /* We can free only the last block allocated.  */
  if (ptr == alloc_last_block)
    {
      /* Since this is rare, we clear the freed block here
	 so that calloc can presume malloc returns cleared memory.  */
      memset (alloc_last_block, '\0', alloc_ptr - alloc_last_block);
      alloc_ptr = alloc_last_block;
    }
}

/* This is only called with the most recent block returned by malloc.  */
void * weak_function
realloc (void *ptr, size_t n)
{
  if (ptr == NULL)
    return malloc (n);
  assert (ptr == alloc_last_block);
  size_t old_size = alloc_ptr - alloc_last_block;
  alloc_ptr = alloc_last_block;
  void *new = malloc (n);
  return new != ptr ? memcpy (new, ptr, old_size) : new;
}

/* Avoid signal frobnication in setjmp/longjmp.  Keeps things smaller.  */

#include <setjmp.h>

int weak_function
__sigjmp_save (sigjmp_buf env, int savemask __attribute__ ((unused)))
{
  env[0].__mask_was_saved = 0;
  return 0;
}

/* Define our own version of the internal function used by strerror.  We
   only provide the messages for some common errors.  This avoids pulling
   in the whole error list.  */

char * weak_function
__strerror_r (int errnum, char *buf, size_t buflen)
{
  char *msg;

  switch (errnum)
    {
    case ENOMEM:
      msg = (char *) "Cannot allocate memory";
      break;
    case EINVAL:
      msg = (char *) "Invalid argument";
      break;
    case ENOENT:
      msg = (char *) "No such file or directory";
      break;
    case EPERM:
      msg = (char *) "Operation not permitted";
      break;
    case EIO:
      msg = (char *) "Input/output error";
      break;
    case EACCES:
      msg = (char *) "Permission denied";
      break;
    default:
      /* No need to check buffer size, all calls in the dynamic linker
	 provide enough space.  */
      buf[buflen - 1] = '\0';
      msg = _itoa (errnum, buf + buflen - 1, 10, 0);
      msg = memcpy (msg - (sizeof ("Error ") - 1), "Error ",
		    sizeof ("Error ") - 1);
      break;
    }

  return msg;
}

void
__libc_fatal (const char *message)
{
  _dl_fatal_printf ("%s", message);
}
rtld_hidden_def (__libc_fatal)

void
__attribute__ ((noreturn))
__chk_fail (void)
{
  _exit (127);
}
rtld_hidden_def (__chk_fail)

#ifndef NDEBUG
/* Define (weakly) our own assert failure function which doesn't use stdio.
   If we are linked into the user program (-ldl), the normal __assert_fail
   defn can override this one.  */

void weak_function
__assert_fail (const char *assertion,
	       const char *file, unsigned int line, const char *function)
{
  _dl_fatal_printf ("\
Inconsistency detected by ld.so: %s: %u: %s%sAssertion `%s' failed!\n",
		    file, line, function ?: "", function ? ": " : "",
		    assertion);

}
rtld_hidden_weak (__assert_fail)

void weak_function
__assert_perror_fail (int errnum,
		      const char *file, unsigned int line,
		      const char *function)
{
  char errbuf[400];
  _dl_fatal_printf ("\
Inconsistency detected by ld.so: %s: %u: %s%sUnexpected error: %s.\n",
		    file, line, function ?: "", function ? ": " : "",
		    __strerror_r (errnum, errbuf, sizeof errbuf));

}
rtld_hidden_weak (__assert_perror_fail)
#endif

unsigned long int weak_function
__strtoul_internal (const char *nptr, char **endptr, int base, int group)
{
  unsigned long int result = 0;
  long int sign = 1;
  unsigned max_digit;

  while (*nptr == ' ' || *nptr == '\t')
    ++nptr;

  if (*nptr == '-')
    {
      sign = -1;
      ++nptr;
    }
  else if (*nptr == '+')
    ++nptr;

  if (*nptr < '0' || *nptr > '9')
    {
      if (endptr != NULL)
	*endptr = (char *) nptr;
      return 0UL;
    }

  assert (base == 0);
  base = 10;
  max_digit = 9;
  if (*nptr == '0')
    {
      if (nptr[1] == 'x' || nptr[1] == 'X')
	{
	  base = 16;
	  nptr += 2;
	}
      else
	{
	  base = 8;
	  max_digit = 7;
	}
    }

  while (1)
    {
      unsigned long int digval;
      if (*nptr >= '0' && *nptr <= '0' + max_digit)
        digval = *nptr - '0';
      else if (base == 16)
        {
	  if (*nptr >= 'a' && *nptr <= 'f')
	    digval = *nptr - 'a' + 10;
	  else if (*nptr >= 'A' && *nptr <= 'F')
	    digval = *nptr - 'A' + 10;
	  else
	    break;
	}
      else
        break;

      if (result > ULONG_MAX / base
	  || (result == ULONG_MAX / base && digval > ULONG_MAX % base))
	{
	  errno = ERANGE;
	  if (endptr != NULL)
	    *endptr = (char *) nptr;
	  return ULONG_MAX;
	}
      result *= base;
      result += digval;
      ++nptr;
    }

  if (endptr != NULL)
    *endptr = (char *) nptr;
  return result * sign;
}


#undef _itoa
/* We always use _itoa instead of _itoa_word in ld.so since the former
   also has to be present and it is never about speed when these
   functions are used.  */
char *
_itoa (unsigned long long int value, char *buflim, unsigned int base,
       int upper_case)
{
  assert (! upper_case);

  do
    *--buflim = _itoa_lower_digits[value % base];
  while ((value /= base) != 0);

  return buflim;
}

/* The '_itoa_lower_digits' variable in libc.so is able to handle bases
   up to 36.  We don't need this here.  */
const char _itoa_lower_digits[16] = "0123456789abcdef";
rtld_hidden_data_def (_itoa_lower_digits)

/* The following is not a complete strsep implementation.  It cannot
   handle empty delimiter strings.  But this isn't necessary for the
   execution of ld.so.  */
#undef strsep
#undef __strsep
char *
__strsep (char **stringp, const char *delim)
{
  char *begin;

  assert (delim[0] != '\0');

  begin = *stringp;
  if (begin != NULL)
    {
      char *end = begin;

      while (*end != '\0' || (end = NULL))
	{
	  const char *dp = delim;

	  do
	    if (*dp == *end)
	      break;
	  while (*++dp != '\0');

	  if (*dp != '\0')
	    {
	      *end++ = '\0';
	      break;
	    }

	  ++end;
	}

      *stringp = end;
    }

  return begin;
}
weak_alias (__strsep, strsep)
strong_alias (__strsep, __strsep_g)
