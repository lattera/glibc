/* Copyright (C) 1999-2018 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.

   As a special exception, if you link the code in this file with
   files compiled with a GNU compiler to produce an executable,
   that does not cause the resulting executable to be covered by
   the GNU Lesser General Public License.  This exception does not
   however invalidate any other reasons why the executable file
   might be covered by the GNU Lesser General Public License.
   This exception applies to code released by its copyright holders
   in files containing the exception.  */

#include <libioP.h>
#include <dlfcn.h>
#include <wchar.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <langinfo.h>
#include <locale/localeinfo.h>
#include <wcsmbs/wcsmbsload.h>
#include <iconv/gconv_int.h>
#include <shlib-compat.h>
#include <sysdep.h>


/* Prototypes of libio's codecvt functions.  */
static enum __codecvt_result do_out (struct _IO_codecvt *codecvt,
				     __mbstate_t *statep,
				     const wchar_t *from_start,
				     const wchar_t *from_end,
				     const wchar_t **from_stop, char *to_start,
				     char *to_end, char **to_stop);
static enum __codecvt_result do_unshift (struct _IO_codecvt *codecvt,
					 __mbstate_t *statep, char *to_start,
					 char *to_end, char **to_stop);
static enum __codecvt_result do_in (struct _IO_codecvt *codecvt,
				    __mbstate_t *statep,
				    const char *from_start,
				    const char *from_end,
				    const char **from_stop, wchar_t *to_start,
				    wchar_t *to_end, wchar_t **to_stop);
static int do_encoding (struct _IO_codecvt *codecvt);
static int do_length (struct _IO_codecvt *codecvt, __mbstate_t *statep,
		      const char *from_start,
		      const char *from_end, size_t max);
static int do_max_length (struct _IO_codecvt *codecvt);
static int do_always_noconv (struct _IO_codecvt *codecvt);


/* The functions used in `codecvt' for libio are always the same.  */
const struct _IO_codecvt __libio_codecvt =
{
  .__codecvt_destr = NULL,		/* Destructor, never used.  */
  .__codecvt_do_out = do_out,
  .__codecvt_do_unshift = do_unshift,
  .__codecvt_do_in = do_in,
  .__codecvt_do_encoding = do_encoding,
  .__codecvt_do_always_noconv = do_always_noconv,
  .__codecvt_do_length = do_length,
  .__codecvt_do_max_length = do_max_length
};


/* Return orientation of stream.  If mode is nonzero try to change
   the orientation first.  */
#undef _IO_fwide
int
_IO_fwide (FILE *fp, int mode)
{
  /* Normalize the value.  */
  mode = mode < 0 ? -1 : (mode == 0 ? 0 : 1);

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_1)
  if (__builtin_expect (&_IO_stdin_used == NULL, 0)
      && (fp == _IO_stdin || fp == _IO_stdout || fp == _IO_stderr))
    /* This is for a stream in the glibc 2.0 format.  */
    return -1;
#endif

  /* The orientation already has been determined.  */
  if (fp->_mode != 0
      /* Or the caller simply wants to know about the current orientation.  */
      || mode == 0)
    return fp->_mode;

  /* Set the orientation appropriately.  */
  if (mode > 0)
    {
      struct _IO_codecvt *cc = fp->_codecvt = &fp->_wide_data->_codecvt;

      fp->_wide_data->_IO_read_ptr = fp->_wide_data->_IO_read_end;
      fp->_wide_data->_IO_write_ptr = fp->_wide_data->_IO_write_base;

      /* Get the character conversion functions based on the currently
	 selected locale for LC_CTYPE.  */
      {
	/* Clear the state.  We start all over again.  */
	memset (&fp->_wide_data->_IO_state, '\0', sizeof (__mbstate_t));
	memset (&fp->_wide_data->_IO_last_state, '\0', sizeof (__mbstate_t));

	struct gconv_fcts fcts;
	__wcsmbs_clone_conv (&fcts);
	assert (fcts.towc_nsteps == 1);
	assert (fcts.tomb_nsteps == 1);

	/* The functions are always the same.  */
	*cc = __libio_codecvt;

	cc->__cd_in.__cd.__nsteps = fcts.towc_nsteps;
	cc->__cd_in.__cd.__steps = fcts.towc;

	cc->__cd_in.__cd.__data[0].__invocation_counter = 0;
	cc->__cd_in.__cd.__data[0].__internal_use = 1;
	cc->__cd_in.__cd.__data[0].__flags = __GCONV_IS_LAST;
	cc->__cd_in.__cd.__data[0].__statep = &fp->_wide_data->_IO_state;

	cc->__cd_out.__cd.__nsteps = fcts.tomb_nsteps;
	cc->__cd_out.__cd.__steps = fcts.tomb;

	cc->__cd_out.__cd.__data[0].__invocation_counter = 0;
	cc->__cd_out.__cd.__data[0].__internal_use = 1;
	cc->__cd_out.__cd.__data[0].__flags
	  = __GCONV_IS_LAST | __GCONV_TRANSLIT;
	cc->__cd_out.__cd.__data[0].__statep = &fp->_wide_data->_IO_state;
      }

      /* From now on use the wide character callback functions.  */
      _IO_JUMPS_FILE_plus (fp) = fp->_wide_data->_wide_vtable;
    }

  /* Set the mode now.  */
  fp->_mode = mode;

  return mode;
}


static enum __codecvt_result
do_out (struct _IO_codecvt *codecvt, __mbstate_t *statep,
	const wchar_t *from_start, const wchar_t *from_end,
	const wchar_t **from_stop, char *to_start, char *to_end,
	char **to_stop)
{
  enum __codecvt_result result;

  struct __gconv_step *gs = codecvt->__cd_out.__cd.__steps;
  int status;
  size_t dummy;
  const unsigned char *from_start_copy = (unsigned char *) from_start;

  codecvt->__cd_out.__cd.__data[0].__outbuf = (unsigned char *) to_start;
  codecvt->__cd_out.__cd.__data[0].__outbufend = (unsigned char *) to_end;
  codecvt->__cd_out.__cd.__data[0].__statep = statep;

  __gconv_fct fct = gs->__fct;
#ifdef PTR_DEMANGLE
  if (gs->__shlib_handle != NULL)
    PTR_DEMANGLE (fct);
#endif

  status = DL_CALL_FCT (fct,
			(gs, codecvt->__cd_out.__cd.__data, &from_start_copy,
			 (const unsigned char *) from_end, NULL,
			 &dummy, 0, 0));

  *from_stop = (wchar_t *) from_start_copy;
  *to_stop = (char *) codecvt->__cd_out.__cd.__data[0].__outbuf;

  switch (status)
    {
    case __GCONV_OK:
    case __GCONV_EMPTY_INPUT:
      result = __codecvt_ok;
      break;

    case __GCONV_FULL_OUTPUT:
    case __GCONV_INCOMPLETE_INPUT:
      result = __codecvt_partial;
      break;

    default:
      result = __codecvt_error;
      break;
    }

  return result;
}


static enum __codecvt_result
do_unshift (struct _IO_codecvt *codecvt, __mbstate_t *statep,
	    char *to_start, char *to_end, char **to_stop)
{
  enum __codecvt_result result;

  struct __gconv_step *gs = codecvt->__cd_out.__cd.__steps;
  int status;
  size_t dummy;

  codecvt->__cd_out.__cd.__data[0].__outbuf = (unsigned char *) to_start;
  codecvt->__cd_out.__cd.__data[0].__outbufend = (unsigned char *) to_end;
  codecvt->__cd_out.__cd.__data[0].__statep = statep;

  __gconv_fct fct = gs->__fct;
#ifdef PTR_DEMANGLE
  if (gs->__shlib_handle != NULL)
    PTR_DEMANGLE (fct);
#endif

  status = DL_CALL_FCT (fct,
			(gs, codecvt->__cd_out.__cd.__data, NULL, NULL,
			 NULL, &dummy, 1, 0));

  *to_stop = (char *) codecvt->__cd_out.__cd.__data[0].__outbuf;

  switch (status)
    {
    case __GCONV_OK:
    case __GCONV_EMPTY_INPUT:
      result = __codecvt_ok;
      break;

    case __GCONV_FULL_OUTPUT:
    case __GCONV_INCOMPLETE_INPUT:
      result = __codecvt_partial;
      break;

    default:
      result = __codecvt_error;
      break;
    }

  return result;
}


static enum __codecvt_result
do_in (struct _IO_codecvt *codecvt, __mbstate_t *statep,
       const char *from_start, const char *from_end, const char **from_stop,
       wchar_t *to_start, wchar_t *to_end, wchar_t **to_stop)
{
  enum __codecvt_result result;

  struct __gconv_step *gs = codecvt->__cd_in.__cd.__steps;
  int status;
  size_t dummy;
  const unsigned char *from_start_copy = (unsigned char *) from_start;

  codecvt->__cd_in.__cd.__data[0].__outbuf = (unsigned char *) to_start;
  codecvt->__cd_in.__cd.__data[0].__outbufend = (unsigned char *) to_end;
  codecvt->__cd_in.__cd.__data[0].__statep = statep;

  __gconv_fct fct = gs->__fct;
#ifdef PTR_DEMANGLE
  if (gs->__shlib_handle != NULL)
    PTR_DEMANGLE (fct);
#endif

  status = DL_CALL_FCT (fct,
			(gs, codecvt->__cd_in.__cd.__data, &from_start_copy,
			 (const unsigned char *) from_end, NULL,
			 &dummy, 0, 0));

  *from_stop = (const char *) from_start_copy;
  *to_stop = (wchar_t *) codecvt->__cd_in.__cd.__data[0].__outbuf;

  switch (status)
    {
    case __GCONV_OK:
    case __GCONV_EMPTY_INPUT:
      result = __codecvt_ok;
      break;

    case __GCONV_FULL_OUTPUT:
    case __GCONV_INCOMPLETE_INPUT:
      result = __codecvt_partial;
      break;

    default:
      result = __codecvt_error;
      break;
    }

  return result;
}


static int
do_encoding (struct _IO_codecvt *codecvt)
{
  /* See whether the encoding is stateful.  */
  if (codecvt->__cd_in.__cd.__steps[0].__stateful)
    return -1;
  /* Fortunately not.  Now determine the input bytes for the conversion
     necessary for each wide character.  */
  if (codecvt->__cd_in.__cd.__steps[0].__min_needed_from
      != codecvt->__cd_in.__cd.__steps[0].__max_needed_from)
    /* Not a constant value.  */
    return 0;

  return codecvt->__cd_in.__cd.__steps[0].__min_needed_from;
}


static int
do_always_noconv (struct _IO_codecvt *codecvt)
{
  return 0;
}


static int
do_length (struct _IO_codecvt *codecvt, __mbstate_t *statep,
	   const char *from_start, const char *from_end, size_t max)
{
  int result;
  const unsigned char *cp = (const unsigned char *) from_start;
  wchar_t to_buf[max];
  struct __gconv_step *gs = codecvt->__cd_in.__cd.__steps;
  size_t dummy;

  codecvt->__cd_in.__cd.__data[0].__outbuf = (unsigned char *) to_buf;
  codecvt->__cd_in.__cd.__data[0].__outbufend = (unsigned char *) &to_buf[max];
  codecvt->__cd_in.__cd.__data[0].__statep = statep;

  __gconv_fct fct = gs->__fct;
#ifdef PTR_DEMANGLE
  if (gs->__shlib_handle != NULL)
    PTR_DEMANGLE (fct);
#endif

  DL_CALL_FCT (fct,
	       (gs, codecvt->__cd_in.__cd.__data, &cp,
		(const unsigned char *) from_end, NULL,
		&dummy, 0, 0));

  result = cp - (const unsigned char *) from_start;

  return result;
}


static int
do_max_length (struct _IO_codecvt *codecvt)
{
  return codecvt->__cd_in.__cd.__steps[0].__max_needed_from;
}
