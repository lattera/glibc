/* Word-wrapping and line-truncating streams.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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

#ifndef __LINEWRAP_H__
#define __LINEWRAP_H__

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <features.h>

#include <string.h>		/* Need size_t.  */

__BEGIN_DECLS

/* We keep this data for each line-wrapping stream.  */
struct line_wrap_data
  {
    size_t lmargin, rmargin;	/* Left and right margins.  */
    ssize_t wmargin;		/* Margin to wrap to, or -1 to truncate.  */

    /* Point in stdio buffer to which we've processed for wrapping, but
       not output.  */
    size_t point_offs;
    /* Output column at POINT_OFFS, or -1 meaning 0 but don't add lmargin.  */
    ssize_t point_col;

    /* Original cookie and hooks from the stream.  */
    void *cookie;
    void (*output) (FILE *, int);
    __io_close_fn *close;
    __io_fileno_fn *fileno;
    __io_seek_fn *seek;
  };

/* Modify STREAM so that it prefixes lines written on it with LMARGIN spaces
   and limits them to RMARGIN columns total.  If WMARGIN >= 0, words that
   extend past RMARGIN are wrapped by replacing the whitespace before them
   with a newline and WMARGIN spaces.  Otherwise, chars beyond RMARGIN are
   simply dropped until a newline.  Returns STREAM after modifying it, or
   NULL if there was an error.  */
FILE *line_wrap_stream (FILE *stream,
			size_t lmargin, size_t rmargin, ssize_t wmargin);

/* Remove the hooks placed in STREAM by `line_wrap_stream'.  */
void line_unwrap_stream (FILE *stream);

/* Returns true if STREAM is line wrapped.  */
extern inline int line_wrapped (FILE *stream);

/* If STREAM is not line-wrapped return -1, else return its left margin.  */
extern size_t line_wrap_lmargin (FILE *stream);

/* If STREAM is not line-wrapped return -1, else set its left margin to
   LMARGIN and return the old value.  */
extern size_t line_wrap_set_lmargin (FILE *stream, size_t lmargin);

/* If STREAM is not line-wrapped return -1, else return its left margin.  */
extern size_t line_wrap_rmargin (FILE *stream);

/* If STREAM is not line-wrapped return -1, else set its right margin to
   RMARGIN and return the old value.  */
extern size_t line_wrap_set_rmargin (FILE *stream, size_t rmargin);

/* If STREAM is not line-wrapped return -1, else return its wrap margin.  */
extern size_t line_wrap_wmargin (FILE *stream);

/* If STREAM is not line-wrapped return -1, else set its left margin to
   WMARGIN and return the old value.  */
extern size_t line_wrap_set_wmargin (FILE *stream, size_t wmargin);

/* If STREAM is not line-wrapped return -1, else return the column number of
   the current output point.  */
extern size_t line_wrap_point (FILE *stream);

#ifdef	__OPTIMIZE__

extern void __line_wrap_output (FILE *, int); /* private */

/* If STREAM is not line-wrapped, return 0.  Otherwise all pending text
   buffered text in STREAM so that the POINT_OFFS field refers to the last
   position in the stdio buffer, and return the line wrap state object for
   STREAM.  Since all text has been processed, this means that (1) the
   POINT_COL field refers to the column at which any new text would be added,
   and (2) any changes to the margin parameters will only affect new text.  */
extern struct line_wrap_data *__line_wrap_update (FILE *stream); /* private */

/* Returns true if STREAM is line wrapped.  */
extern inline int
line_wrapped (FILE *stream)
{
  return (stream->__room_funcs.__output == &__line_wrap_output);
}

/* If STREAM is not line-wrapped return -1, else return its left margin.  */
extern inline size_t
line_wrap_lmargin (FILE *stream)
{
  if (! line_wrapped (stream))
    return -1;
  return ((struct line_wrap_data *)stream->__cookie)->lmargin;
}

/* If STREAM is not line-wrapped return -1, else set its left margin to
   LMARGIN and return the old value.  */
extern inline size_t
line_wrap_set_lmargin (FILE *stream, size_t lmargin)
{
  struct line_wrap_data *d = __line_wrap_update (stream);
  if (d)
    {
      size_t old = d->lmargin;
      d->lmargin = lmargin;
      return old;
    }
  else
    return -1;
}

/* If STREAM is not line-wrapped return -1, else return its left margin.  */
extern inline size_t
line_wrap_rmargin (FILE *stream)
{
  if (! line_wrapped (stream))
    return -1;
  return ((struct line_wrap_data *)stream->__cookie)->rmargin;
}

/* If STREAM is not line-wrapped return -1, else set its right margin to
   RMARGIN and return the old value.  */
extern inline size_t
line_wrap_set_rmargin (FILE *stream, size_t rmargin)
{
  struct line_wrap_data *d = __line_wrap_update (stream);
  if (d)
    {
      size_t old = d->rmargin;
      d->rmargin = rmargin;
      return old;
    }
  else
    return -1;
}

/* If STREAM is not line-wrapped return -1, else return its wrap margin.  */
extern inline size_t
line_wrap_wmargin (FILE *stream)
{
  if (! line_wrapped (stream))
    return -1;
  return ((struct line_wrap_data *)stream->__cookie)->wmargin;
}

/* If STREAM is not line-wrapped return -1, else set its left margin to
   WMARGIN and return the old value.  */
extern inline size_t
line_wrap_set_wmargin (FILE *stream, size_t wmargin)
{
  struct line_wrap_data *d = __line_wrap_update (stream);
  if (d)
    {
      size_t old = d->wmargin;
      d->wmargin = wmargin;
      return old;
    }
  else
    return -1;
}

/* If STREAM is not line-wrapped return -1, else return the column number of
   the current output point.  */
extern inline size_t
line_wrap_point (FILE *stream)
{
  struct line_wrap_data *d = __line_wrap_update (stream);
  return d ? (d->point_col >= 0 ? d->point_col : 0) : -1;
}

#endif /* Optimizing.  */

__END_DECLS

#endif /* __LINEWRAP_H__ */
