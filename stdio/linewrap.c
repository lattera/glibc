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

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <linewrap.h>

void __line_wrap_output (FILE *, int);

/* Install our hooks into a stream.  */
static inline void
wrap_stream (FILE *stream, struct line_wrap_data *d)
{
  static __io_close_fn lwclose;
  static __io_fileno_fn lwfileno;

  stream->__cookie = d;
  stream->__room_funcs.__output = &__line_wrap_output;
  stream->__io_funcs.__close = &lwclose;
  stream->__io_funcs.__fileno = &lwfileno;
  stream->__io_funcs.__seek = NULL; /* Cannot seek.  */
}

/* Restore a stream to its original state.  */
static inline void
unwrap_stream (FILE *stream, struct line_wrap_data *d)
{
  stream->__cookie = d->cookie;
  stream->__room_funcs.__output = d->output;
  stream->__io_funcs.__close = d->close;
  stream->__io_funcs.__fileno = d->fileno;
  stream->__io_funcs.__seek = d->seek;
}

/* If WRAPPER_COOKIE points to a 0 pointer, then STREAM is assumed to be
   wrapped, and will be unwrapped, storing the wrapper cookie into
   WRAPPER_COOKIE.  Otherwise, nothing is done.  */
static inline void
ensure_unwrapped (FILE *stream, struct line_wrap_data **wrapper_cookie)
{
  if (*wrapper_cookie == 0)
    {
      *wrapper_cookie = stream->__cookie;
      unwrap_stream (stream, *wrapper_cookie);
    }
}

/* If WRAPPER_COOKIE points to a non-0 pointer, then STREAM is assumed to
   *have been unwrapped with ensure_unwrapped, will be wrapped with
   *WRAPPER_COOKIE, and *WRAPPER_COOKIE zeroed.  Otherwise, nothing is done. */
static inline void
ensure_wrapped (FILE *stream, struct line_wrap_data **wrapper_cookie)
{
  if (*wrapper_cookie)
    {
      wrap_stream (stream, *wrapper_cookie);
      *wrapper_cookie = 0;
    }
}

/* Cookie io functions that might get called on a wrapped stream.
   Must pass the original cookie to the original functions.  */

static int
lwclose (void *cookie)
{
  struct line_wrap_data *d = cookie;
  return (*d->close) (d->cookie);
}

static int
lwfileno (void *cookie)
{
  struct line_wrap_data *d = cookie;
  return (*d->fileno) (d->cookie);
}

/* Process STREAM's buffer so that line wrapping is done from POINT_OFFS to
   the end of its buffer.  If WRAPPER_COOKIE is 0, and it's necessary to
   flush some data, STREAM is unwrapped, and the line wrap stdio cookie
   stored in WRAPPER_COOKIE; otherwise, stream is assumed to already be
   unwrapped, and WRAPPER_COOKIE to point to the line wrap data.  Returns C
   or EOF if C was output.  */
static inline int
lwupdate (FILE *stream, int c, struct line_wrap_data **wrapper_cookie)
{
  char *buf, *nl;
  size_t len;
  struct line_wrap_data *d = *wrapper_cookie ?: stream->__cookie;

  /* Scan the buffer for newlines.  */
  buf = stream->__buffer + d->point_offs;
  while ((buf < stream->__bufp || (c != EOF && c != '\n')) && !stream->__error)
    {
      size_t r;

      if (d->point_col == 0 && d->lmargin != 0)
	{
	  /* We are starting a new line.  Print spaces to the left margin.  */
	  const size_t pad = d->lmargin;
	  if (stream->__bufp + pad < stream->__put_limit)
	    {
	      /* We can fit in them in the buffer by moving the
		 buffer text up and filling in the beginning.  */
	      memmove (buf + pad, buf, stream->__bufp - buf);
	      stream->__bufp += pad; /* Compensate for bigger buffer. */
	      memset (buf, ' ', pad); /* Fill in the spaces.  */
	      buf += pad; /* Don't bother searching them.  */
	    }
	  else
	    {
	      /* No buffer space for spaces.  Must flush.  */
	      size_t i;
	      char *olimit;

	      ensure_unwrapped (stream, wrapper_cookie);

	      len = stream->__bufp - buf;
	      olimit = stream->__put_limit;
	      stream->__bufp = stream->__put_limit = buf;
	      for (i = 0; i < pad; ++i)
		(*d->output) (stream, ' ');
	      stream->__put_limit = olimit;
	      memmove (stream->__bufp, buf, len);
	      stream->__bufp += len;
	    }
	  d->point_col = pad;
	}

      len = stream->__bufp - buf;
      nl = memchr (buf, '\n', len);

      if (d->point_col < 0)
	d->point_col = 0;

      if (!nl)
	{
	  /* The buffer ends in a partial line.  */

	  if (d->point_col + len + (c != EOF && c != '\n') < d->rmargin)
	    {
	      /* The remaining buffer text is a partial line and fits
		 within the maximum line width.  Advance point for the
		 characters to be written and stop scanning.  */
	      d->point_col += len;
	      break;
	    }
	  else
	    /* Set the end-of-line pointer for the code below to
	       the end of the buffer.  */
	    nl = stream->__bufp;
	}
      else if ((size_t) d->point_col + (nl - buf) < d->rmargin)
	{
	  /* The buffer contains a full line that fits within the maximum
	     line width.  Reset point and scan the next line.  */
	  d->point_col = 0;
	  buf = nl + 1;
	  continue;
	}

      /* This line is too long.  */
      r = d->rmargin - 1;

      if (d->wmargin < 0)
	{
	  /* Truncate the line by overwriting the excess with the
	     newline and anything after it in the buffer.  */
	  if (nl < stream->__bufp)
	    {
	      memmove (buf + (r - d->point_col), nl, stream->__bufp - nl);
	      stream->__bufp -= buf + (r - d->point_col) - nl;
	      /* Reset point for the next line and start scanning it.  */
	      d->point_col = 0;
	      buf += r + 1; /* Skip full line plus \n. */
	    }
	  else
	    {
	      /* The buffer ends with a partial line that is beyond the
		 maximum line width.  Advance point for the characters
		 written, and discard those past the max from the buffer.  */
	      d->point_col += len;
	      stream->__bufp -= d->point_col - r;
	      if (c != '\n')
		/* Swallow the extra character too.  */
		c = EOF;
	      break;
	    }
	}
      else
	{
	  /* Do word wrap.  Go to the column just past the maximum line
	     width and scan back for the beginning of the word there.
	     Then insert a line break.  */

	  char *p, *nextline;
	  int i;

	  p = buf + (r + 1 - d->point_col);
	  while (p >= buf && !isblank (*p))
	    --p;
	  nextline = p + 1;	/* This will begin the next line.  */

	  if (nextline > buf)
	    {
	      /* Swallow separating blanks.  */
	      do
		--p;
	      while (isblank (*p));
	      nl = p + 1;	/* The newline will replace the first blank. */
	    }
	  else
	    {
	      /* A single word that is greater than the maximum line width.
		 Oh well.  Put it on an overlong line by itself.  */
	      p = buf + (r + 1 - d->point_col);
	      /* Find the end of the long word.  */
	      do
		++p;
	      while (p < nl && !isblank (*p));
	      if (p == nl)
		{
		  /* It already ends a line.  No fussing required.  */
		  d->point_col = 0;
		  buf = nl + 1;
		  continue;
		}
	      /* We will move the newline to replace the first blank.  */
	      nl = p;
	      /* Swallow separating blanks.  */
	      do
		++p;
	      while (isblank (*p));
	      /* The next line will start here.  */
	      nextline = p;
	    }

	  /* Temporarily reset bufp to include just the first line.  */
	  stream->__bufp = nl;
	  if (nextline - (nl + 1) < d->wmargin)
	    /* The margin needs more blanks than we removed.
	       Output the first line so we can use the space.  */
	    {
	      ensure_unwrapped (stream, wrapper_cookie);
	      (*d->output) (stream, '\n');
	    }
	  else
	    /* We can fit the newline and blanks in before
	       the next word.  */
	    *stream->__bufp++ = '\n';

	  /* Reset the counter of what has been output this line.  If wmargin
	     is 0, we want to avoid the lmargin getting added, so we set
	     point_col to a magic value of -1 in that case.  */
	  d->point_col = d->wmargin ? d->wmargin : -1;

	  /* Add blanks up to the wrap margin column.  */
	  for (i = 0; i < d->wmargin; ++i)
	    *stream->__bufp++ = ' ';

	  /* Copy the tail of the original buffer into the current buffer
	     position.  */
	  if (stream->__bufp != nextline)
	    memmove (stream->__bufp, nextline, buf + len - nextline);
	  len -= nextline - buf;

	  /* Continue the scan on the remaining lines in the buffer.  */
	  buf = stream->__bufp;

	  /* Restore bufp to include all the remaining text.  */
	  stream->__bufp += len;
	}
    }

  /* Remember that we've scanned as far as the end of the buffer.  */
  d->point_offs = stream->__bufp - stream->__buffer;

  return c;
}

/* This function is called when STREAM must be flushed.
   C is EOF or a character to be appended to the buffer contents.  */
void
__line_wrap_output (FILE *stream, int c)
{
  struct line_wrap_data *d = 0;

  c = lwupdate (stream, c, &d);

  if (!stream->__error)
    {
      ensure_unwrapped (stream, &d);
      (*d->output) (stream, c);
      d->point_offs = 0;	/* The buffer now holds nothing.  */
      if (c == '\n')
	d->point_col = 0;
      else if (c != EOF)
	++d->point_col;
    }

  ensure_wrapped (stream, &d);
}

/* Modify STREAM so that it prefixes lines written on it with LMARGIN spaces
   and limits them to RMARGIN columns total.  If WMARGIN >= 0, words that
   extend past RMARGIN are wrapped by replacing the whitespace before them
   with a newline and WMARGIN spaces.  Otherwise, chars beyond RMARGIN are
   simply dropped until a newline.  Returns STREAM after modifying it, or
   NULL if there was an error.  */
FILE *
line_wrap_stream (FILE *stream, size_t lmargin, size_t rmargin, ssize_t wmargin)
{
  struct line_wrap_data *d = malloc (sizeof *d);

  if (!d)
    return NULL;

  /* Ensure full setup before we start tweaking.  */
  fflush (stream);

  /* Initialize our wrapping state.  */
  d->point_col = 0;
  d->point_offs = 0;

  /* Save the original cookie and output and close hooks.  */
  d->cookie = stream->__cookie;
  d->output = stream->__room_funcs.__output;
  d->close = stream->__io_funcs.__close;
  d->fileno = stream->__io_funcs.__fileno;
  d->seek = stream->__io_funcs.__seek;

  /* Take over the stream.  */
  wrap_stream (stream, d);

  /* Line-wrapping streams are normally line-buffered.  This is not
     required, just assumed desired.  The wrapping feature should continue
     to work if the stream is switched to full or no buffering.  */
  stream->__linebuf = 1;

  d->lmargin = lmargin;
  d->rmargin = rmargin;
  d->wmargin = wmargin;

  return stream;
}

/* Remove the hooks placed in STREAM by `line_wrap_stream'.  */
void
line_unwrap_stream (FILE *stream)
{
  struct line_wrap_data *d = stream->__cookie;
  unwrap_stream (stream, d);
  free (d);
}

/* Functions on wrapped streams.  */

/* Returns true if STREAM is line wrapped.  */
inline int
line_wrapped (FILE *stream)
{
  return (stream->__room_funcs.__output == &__line_wrap_output);
}

/* If STREAM is not line-wrapped, return 0.  Otherwise all pending text
   buffered text in STREAM so that the POINT_OFFS field refers to the last
   position in the stdio buffer, and return the line wrap state object for
   STREAM.  Since all text has been processed, this means that (1) the
   POINT_COL field refers to the column at which any new text would be added,
   and (2) any changes to the margin parameters will only affect new text.  */
struct line_wrap_data *
__line_wrap_update (FILE *stream)
{
  if (line_wrapped (stream))
    {
      struct line_wrap_data *d = stream->__cookie, *wc = 0;

      if (stream->__linebuf_active)
	/* This is an active line-buffered stream, so its put-limit is set to
	   the beginning of the buffer in order to force a __flshfp call on
	   each putc (see below).  We undo this hack here (by setting the
	   limit to the end of the buffer) to simplify the interface with the
	   output-room function.  */
	stream->__put_limit = stream->__buffer + stream->__bufsize;

      lwupdate (stream, EOF, &wc);

      if (stream->__linebuf)
	{
	  /* This is a line-buffered stream, and it is now ready to do some
	     output.  We call this an "active line-buffered stream".  We set
	     the put_limit to the beginning of the buffer, so the next `putc'
	     call will force a call to flshfp.  Setting the linebuf_active
	     flag tells the code above (on the next call) to undo this
	     hackery.  */
	  stream->__put_limit = stream->__buffer;
	  stream->__linebuf_active = 1;
	}

      ensure_wrapped (stream, &wc);

      return d;
    }
  else
    return 0;
}

/* If STREAM is not line-wrapped return -1, else return its left margin.  */
inline size_t
line_wrap_lmargin (FILE *stream)
{
  if (! line_wrapped (stream))
    return -1;
  return ((struct line_wrap_data *)stream->__cookie)->lmargin;
}

/* If STREAM is not line-wrapped return -1, else set its left margin to
   LMARGIN and return the old value.  */
inline size_t
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
inline size_t
line_wrap_rmargin (FILE *stream)
{
  if (! line_wrapped (stream))
    return -1;
  return ((struct line_wrap_data *)stream->__cookie)->rmargin;
}

/* If STREAM is not line-wrapped return -1, else set its right margin to
   RMARGIN and return the old value.  */
inline size_t
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
inline size_t
line_wrap_wmargin (FILE *stream)
{
  if (! line_wrapped (stream))
    return -1;
  return ((struct line_wrap_data *)stream->__cookie)->wmargin;
}

/* If STREAM is not line-wrapped return -1, else set its left margin to
   WMARGIN and return the old value.  */
inline size_t
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
inline size_t
line_wrap_point (FILE *stream)
{
  struct line_wrap_data *d = __line_wrap_update (stream);
  return d ? (d->point_col >= 0 ? d->point_col : 0) : -1;
}

#ifdef TEST
int
main (int argc, char **argv)
{
  int c;
  puts ("stopme");
  line_wrap_stream (stdout, atoi (argv[1]), atoi (argv[2] ?: "-1"));
  while ((c = getchar()) != EOF) putchar (c);
  return 0;
}
#endif
