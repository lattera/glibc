/* Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Bruno Haible <haible@clisp.cons.org>, 2000.

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

/* Create a table from CHARSET to Unicode.
   This is a good test for CHARSET's iconv() module, in particular the
   FROM_LOOP BODY macro.  */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <errno.h>

/* Converts a byte buffer to a hexadecimal string.  */
static const char*
hexbuf (unsigned char buf[], unsigned int buflen)
{
  static char msg[50];

  switch (buflen)
    {
    case 1:
      sprintf (msg, "0x%02X", buf[0]);
      break;
    case 2:
      sprintf (msg, "0x%02X%02X", buf[0], buf[1]);
      break;
    case 3:
      sprintf (msg, "0x%02X%02X%02X", buf[0], buf[1], buf[2]);
      break;
    case 4:
      sprintf (msg, "0x%02X%02X%02X%02X", buf[0], buf[1], buf[2], buf[3]);
      break;
    default:
      abort ();
    }
  return msg;
}

/* Attempts to convert a byte buffer BUF (BUFLEN bytes) to OUT (6 bytes)
   using the conversion descriptor CD.  Returns the number of written bytes,
   or 0 if ambiguous, or -1 if invalid.  */
static int
try (iconv_t cd, unsigned char buf[], unsigned int buflen, unsigned char *out)
{
  const char *inbuf = (const char *) buf;
  size_t inbytesleft = buflen;
  char *outbuf = (char *) out;
  size_t outbytesleft = 6;
  size_t result = iconv (cd,
			 (char **) &inbuf, &inbytesleft,
			 &outbuf, &outbytesleft);
  if (result == (size_t)(-1))
    {
      if (errno == EILSEQ)
	{
	  return -1;
	}
      else if (errno == EINVAL)
	{
	  return 0;
	}
      else
	{
	  int saved_errno = errno;
	  fprintf (stderr, "%s: iconv error: ", hexbuf (buf, buflen));
	  errno = saved_errno;
	  perror ("");
	  exit (1);
	}
    }
  else
    {
      if (inbytesleft != 0)
	{
	  fprintf (stderr, "%s: inbytes = %ld, outbytes = %ld\n",
		   hexbuf (buf, buflen),
		   (long) (buflen - inbytesleft),
		   (long) (6 - outbytesleft));
	  exit (1);
	}
      return 6 - outbytesleft;
    }
}

/* Returns the out[] buffer as a Unicode value.  */
static unsigned int
utf8_decode (const unsigned char *out, unsigned int outlen)
{
  return (outlen==1 ? out[0] :
	  outlen==2 ? ((out[0] & 0x1f) << 6) + (out[1] & 0x3f) :
	  outlen==3 ? ((out[0] & 0x0f) << 12) + ((out[1] & 0x3f) << 6) + (out[2] & 0x3f) :
	  outlen==4 ? ((out[0] & 0x07) << 18) + ((out[1] & 0x3f) << 12) + ((out[2] & 0x3f) << 6) + (out[3] & 0x3f) :
	  outlen==5 ? ((out[0] & 0x03) << 24) + ((out[1] & 0x3f) << 18) + ((out[2] & 0x3f) << 12) + ((out[3] & 0x3f) << 6) + (out[4] & 0x3f) :
	  outlen==6 ? ((out[0] & 0x01) << 30) + ((out[1] & 0x3f) << 24) + ((out[2] & 0x3f) << 18) + ((out[3] & 0x3f) << 12) + ((out[4] & 0x3f) << 6) + (out[5] & 0x3f) :
	  0xfffd);
}

int
main (int argc, char *argv[])
{
  const char *charset;
  iconv_t cd;

  if (argc != 2)
    {
      fprintf (stderr, "Usage: tst-table-to charset\n");
      exit (1);
    }
  charset = argv[1];

  cd = iconv_open ("UTF-8", charset);
  if (cd == (iconv_t)(-1))
    {
      perror ("iconv_open");
      exit (1);
    }

  {
    unsigned char out[6];
    unsigned char buf[4];
    unsigned int i0, i1, i2, i3;
    int result;

    for (i0 = 0; i0 < 0x100; i0++)
      {
	buf[0] = i0;
	result = try (cd, buf, 1, out);
	if (result < 0)
	  {
	  }
	else if (result > 0)
	  {
	    printf ("0x%02X\t0x%04X\n",
		    i0, utf8_decode (out, result));
	  }
	else
	  {
	    for (i1 = 0; i1 < 0x100; i1++)
	      {
		buf[1] = i1;
		result = try (cd, buf, 2, out);
		if (result < 0)
		  {
		  }
		else if (result > 0)
		  {
		    printf ("0x%02X%02X\t0x%04X\n",
			    i0, i1, utf8_decode (out, result));
		  }
		else
		  {
		    for (i2 = 0; i2 < 0x100; i2++)
		      {
			buf[2] = i2;
			result = try (cd, buf, 3, out);
			if (result < 0)
			  {
			  }
			else if (result > 0)
			  {
			    printf ("0x%02X%02X%02X\t0x%04X\n",
				    i0, i1, i2, utf8_decode (out, result));
			  }
			else if (strcmp (charset, "UTF-8"))
			  {
			    for (i3 = 0; i3 < 0x100; i3++)
			      {
				buf[3] = i3;
				result = try (cd, buf, 4, out);
				if (result < 0)
				  {
				  }
				else if (result > 0)
				  {
				    printf ("0x%02X%02X%02X%02X\t0x%04X\n",
					    i0, i1, i2, i3,
					    utf8_decode (out, result));
				  }
				else
				  {
				    fprintf (stderr,
					     "%s: incomplete byte sequence\n",
					     hexbuf (buf, 4));
				    exit (1);
				  }
			      }
			  }
		      }
		  }
	      }
	  }
      }
  }

  if (iconv_close (cd) < 0)
    {
      perror ("iconv_close");
      exit (1);
    }

  if (ferror (stdin) || fflush (stdout) || ferror (stdout))
    {
      fprintf (stderr, "I/O error\n");
      exit (1);
    }

  return 0;
}
