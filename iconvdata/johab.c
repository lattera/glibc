/* Mapping tables for JOHAB handling.
   Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jungshik Shin <jshin@pantheon.yale.edu>, 1998.

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

#include <gconv.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include <ksc5601.h>

/* Direction of the transformation.  */
static int to_johab_object;
static int from_johab_object;

/* The table for Bit pattern to Hangul Jamo
   5 bits each are used to encode
   leading consonants(19 + 1 filler), medial vowels(21 + 1 filler)
   and trailing consonants(27 + 1 filler).

   KS C 5601-1992 Annex 3 Table 2
   0 : Filler, -1: invalid, >= 1 : valid

 */
const int init[32] =
{
  -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
  19, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
const int mid[32] =
{
  -1, -1, 0, 1, 2, 3, 4, 5,
  -1, -1, 6, 7, 8, 9, 10, 11,
  -1, -1, 12, 13, 14, 15, 16, 17,
  -1, -1, 18, 19, 20, 21, -1, -1
};
const int final[32] =
{
  -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
  -1, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, -1, -1
};

/*
   Hangul Jamo in Johab to Unicode 2.0 : Unicode 2.0
   defines 51 Hangul Compatibility Jamos in the block [0x3131,0x314e]

   It's to be considered later which Jamo block to use, Compatibility
   block [0x3131,0x314e] or Hangul Conjoining Jamo block, [0x1100,0x11ff]

 */
const wchar_t init_to_ucs[19] =
{
  0x3131, 0x3132, 0x3134, 0x3137, 0x3138, 0x3139, 0x3141, 0x3142,
  0x3143, 0x3145, 0x3146, 0x3147, 0x3148, 0x3149, 0x314a, 0x314b,
  0x314c, 0x314d, 0x314e
};

const wchar_t final_to_ucs[27] =
{
  L'\0', L'\0', 0x3133, L'\0', 0x3135, 0x3136, L'\0', L'\0',
  0x313a, 0x313b, 0x314c, 0x313d, 0x313e, 0x313f,
  0x3140, L'\0', L'\0', 0x3144, L'\0', L'\0', L'\0',
  L'\0', L'\0', L'\0', L'\0', L'\0', L'\0'
};

/* The following three arrays are used to convert
   precomposed Hangul syllables in [0xac00,0xd???]
   to Jamo bit patterns for Johab encoding

   cf. : KS C 5601-1992, Annex3 Table 2

   Arrays are used to speed up things although it's possible
   to get the same result arithmetically.

 */
const int init_to_bit[19] =
{
  0x8800, 0x8c00, 0x9000, 0x9400, 0x9800, 0x9c00,
  0xa000, 0xa400, 0xa800, 0xac00, 0xb000, 0xb400,
  0xb800, 0xbc00, 0xc000, 0xc400, 0xc800, 0xcc00,
  0xd000
};

const int mid_to_bit[21] =
{
          0x0060, 0x0080, 0x00a0, 0x00c0, 0x00e0,
  0x0140, 0x0160, 0x0180, 0x01a0, 0x01c0, 0x1e0,
  0x0240, 0x0260, 0x0280, 0x02a0, 0x02c0, 0x02e0,
  0x0340, 0x0360, 0x0380, 0x03a0
};

const int final_to_bit[28] =
{
  1, 2, 3, 4, 5, 6, 7, 8, 9, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
  0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d
};

/* The conversion table from
   UCS4 Hangul Compatibility Jamo in [0x3131,0x3163]
   to Johab

   cf. 1. KS C 5601-1992 Annex 3 Table 2
   2. Unicode 2.0 manual

 */
const uint16_t jamo_from_ucs_table[51] =
{
  0x8841, 0x8c41,
  0x8444,
  0x9041,
  0x8446, 0x8447,
  0x9441, 0x9841, 0x9c41,
  0x844a, 0x844b, 0x844c, 0x844d, 0x884e, 0x884f, 0x8450,
  0xa041, 0xa441, 0xa841,
  0x8454,
  0xac41, 0xb041, 0xb441, 0xb841, 0xbc41,
  0xc041, 0xc441, 0xc841, 0xca41, 0xd041,
  0x8461, 0x8481, 0x84a1, 0x84c1, 0x84e1,
  0x8541, 0x8561, 0x8581, 0x85a1, 0x85c1, 0x85e1,
  0x8641, 0x8661, 0x8681, 0x86a1, 0x86c1, 0x86e1,
  0x8741, 0x8761, 0x8781, 0x87a1
};


static inline wchar_t
johab_sym_hanja_to_ucs (int idx, int c1, int c2)
{
  if (idx <= 0xdefe)
    return (wchar_t) ksc5601_sym_to_ucs[(c1 - 0xd9) * 188 + c2
					- (c2 > 0x90 ? 0x43 : 0x31)];
  else
    return (wchar_t) ksc5601_hanja_to_ucs[(c1 - 0xe0) * 188 + c2
					  - (c2 > 0x90 ? 0x43 : 0x31)];
}

static uint16_t
johab_hanja_from_ucs (wchar_t ch)
{

  uint16_t idx;
  if (ucs4_to_ksc5601_hanja (ch, &idx))
    {
      int idx1, idx2;
      /* Hanja begins at the 42th row. 42=0x2a : 0x2a + 0x20 = 0x4a.  */
      idx1 = idx / 256 - 0x4a;
      idx2 = idx % 256 + 0x80;

      return ((idx1 / 2) * 256 + 0xe000 + idx2
	      + (idx1 % 2 ? 0 :  (idx2 > 0xee ? 0x43 : 0x31) - 0xa1));
    }
  else
    return 0;
}

static uint16_t
johab_sym_from_ucs (wchar_t ch)
{
  uint16_t idx;
  if (ucs4_to_ksc5601_sym (ch, &idx))
    {
      int idx1, idx2;

      idx1 = idx / 256 - 0x21;
      idx2 = idx % 256 + 0x80;

      return ((idx1 / 2) * 256 + 0xd900 + idx2
	      + (idx1 % 2 ? 0 : (idx2 > 0xee ? 0x43 : 0x31) - 0xa1));
    }
  else
    return 0;
}



static inline void
johab_from_ucs4 (wchar_t ch, unsigned char *cp)
{
  if (ch >= 0x7f)
    {
      int idx;

      if (ch >= 0xac00 && ch <= 0xd7a3)
	{
	  ch -= 0xac00;
	  idx = init_to_bit[ch / 588];  /* 21*28 = 588 */
	  idx += mid_to_bit[(ch / 28) % 21];  /* (ch % (21 * 28)) / 28 */
	  idx += final_to_bit[ch %  28]; /* (ch % (21 * 28)) % 28 */
	}
      /* KS C 5601-1992 Annex 3 regards  0xA4DA(Hangul Filler : U3164)
         as symbol */
      else if (ch >= 0x3131 && ch <= 0x3163)
	idx = jamo_from_ucs_table[ch - 0x3131];
      else if (ch >= 0x4e00 && ch <= 0x9fa5
	       || ch >= 0xf900 && ch <= 0xfa0b)
	idx = johab_hanja_from_ucs (ch);
      /*       Half-width Korean Currency Won Sign
	       else if ( ch == 0x20a9 )
	       idx = 0x5c00;
      */
      else
	idx = johab_sym_from_ucs (ch);

      *cp = (char) (idx / 256);
      *(cp + 1) = (char) (idx & 0xff);

    }
  else
    {
      *cp = (char) (0x7f & ch);
      *(cp + 1) = (char) 0;
    }

}


int
gconv_init (struct gconv_step *step)
{
  /* Determine which direction.  */
  if (strcasestr (step->from_name, "JOHAB") != NULL)
    step->data = &from_johab_object;
  else if (strcasestr (step->to_name, "JOHAB") != NULL)
    step->data = &to_johab_object;
  else
    return GCONV_NOCONV;

  return GCONV_OK;
}


void
gconv_end (struct gconv_step *data)
{
  /* Nothing to do.  */
}


int
gconv (struct gconv_step *step, struct gconv_step_data *data,
       const char *inbuf, size_t *inbufsize, size_t * written, int do_flush)
{
  struct gconv_step *next_step = step + 1;
  struct gconv_step_data *next_data = data + 1;
  gconv_fct fct = next_step->fct;
  size_t do_write;
  int result;

  /* If the function is called with no input this means we have to reset
     to the initial state.  The possibly partly converted input is
     dropped.  */
  if (do_flush)
    {
      do_write = 0;

      /* Call the steps down the chain if there are any.  */
      if (data->is_last)
	result = GCONV_OK;
      else
	{
	  struct gconv_step *next_step = step + 1;
	  struct gconv_step_data *next_data = data + 1;

	  result = (*fct) (next_step, next_data, NULL, 0, written, 1);

	  /* Clear output buffer.  */
	  data->outbufavail = 0;
	}
    }
  else
    {
      do_write = 0;

      do
	{
	  result = GCONV_OK;

	  if (step->data == &from_johab_object)
	    {
	      size_t inchars = *inbufsize;
	      size_t outwchars = data->outbufavail;
	      char *outbuf = data->outbuf;
	      size_t cnt = 0;

	      while (cnt < inchars
		     && (outwchars + sizeof (wchar_t) <= data->outbufsize))
		{
		  int inchar = (unsigned char) inbuf[cnt];
		  wchar_t ch;
		  /* half-width Korean Currency WON sign
		     if (inchar == 0x5c)
		     ch =  0x20a9;
		     else if (inchar < 0x7f)
		     ch = (wchar_t) inchar;
		  */
		  if (inchar < 0x7f)
		    ch = (wchar_t) inchar;

		  /* Johab : 1. Hangul
		     1st byte : 0x84-0xd3
		     2nd byte : 0x41-0x7e, 0x81-0xfe
		     2. Hanja & Symbol  :
		     1st byte : 0xd8-0xde, 0xe0-0xf9
		     2nd byte : 0x31-0x7e, 0x91-0xfe
		     0xd831-0xd87e and 0xd891-0xd8fe are user-defined area */

		  else if (inchar > 0xf9 || inchar == 0xdf
			   || (inchar > 0x7e && inchar < 0x84)
			   || (inchar > 0xd3 && inchar < 0xd9))
		    /* These are illegal.  */
		    ch = L'\0';
		  else
		    {
		      /* Two-byte character.  First test whether the next
		         character is also available.  */
		      int inchar2;
		      int idx;

		      if (cnt + 1 >= inchars)
			{
			  /* The second character is not available.  Store
			     the intermediate result.  */
			  result = GCONV_INCOMPLETE_INPUT;
			  break;
			}

		      inchar2 = (unsigned char) inbuf[++cnt];
		      idx = inchar * 256 + inchar2;
		      if (inchar <= 0xd3)
			{	/* Hangul */
			  int i, m, f;
			  i = init[(idx & 0x7c00) >> 10];
			  m = mid[(idx & 0x03e0) >> 5];
			  f = final[idx & 0x001f];
			  if (i == -1 || m == -1 || f == -1)
			    /* This is illegal.  */
			    ch = L'\0';
			  else if (i > 0 && m > 0)
			    ch = ((i - 1) * 21 + (m - 1)) * 28 + f + 0xac00;
			  else if (i > 0 && m == 0 & f == 0)
			    ch = init_to_ucs[i - 1];
			  else if (i == 0 && m > 0 & f == 0)
			    ch = 0x314e + m;	/* 0x314f + m - 1 */
			  else if (i == 0 && m == 0 & f > 0)
			    ch = final_to_ucs[f - 1];	/* round trip?? */
			  else
			    /* This is illegal.  */
			    ch = L'\0';
			}
		      else
			{
			  if (inchar2 < 0x31
			      || (inchar2 > 0x7e && inchar2 < 0x91)
			      || inchar2 == 0xff)
			    /* This is illegal.  */
			    ch = L'\0';
			  else if (inchar == 0xda
				   && inchar2 > 0xa0 && inchar2 < 0xd4)
			    /* This is illegal.  */
			    /* Modern Hangul Jaso is defined elsewhere
			       in Johab */
			    ch = L'\0';
			  else
			    {
			      ch = johab_sym_hanja_to_ucs (idx, inchar,
							   inchar2);
			      /*                if (idx <= 0xdefe)
			         ch = ksc5601_sym_to_ucs[(inchar - 0xd9) * 192
			         + inchar2
			         -  (inchar2>0x90 ? 0x43 : 0x31)];

			         else
			         ch = ksc5601_hanja_to_ucs[(inchar - 0xe0) *192
			         + inchar2
			         -  (inchar2>0x90 ? 0x43 : 0x31)];
			       */
			    }
			}

		      if (ch == L'\0')
			--cnt;
		    }

		  if (ch == L'\0' && inbuf[cnt] != '\0')
		    {
		      /* This is an illegal character.  */
		      result = GCONV_ILLEGAL_INPUT;
		      break;
		    }

		  *((wchar_t *) (outbuf + outwchars)) = ch;
		  ++do_write;
		  outwchars += sizeof (wchar_t);
		  ++cnt;
		}
	      *inbufsize -= cnt;
	      data->outbufavail = outwchars;
	    }
	  else
	    {
	      size_t inwchars = *inbufsize;
	      size_t outchars = data->outbufavail;
	      char *outbuf = data->outbuf;
	      size_t cnt = 0;
	      int extra = 0;

	      while (inwchars >= cnt + sizeof (wchar_t)
		     && outchars < data->outbufsize)
		{
		  wchar_t ch = *((wchar_t *) (inbuf + cnt));
		  unsigned char cp[2];
		  /*
		    if (ch >= (sizeof (from_ucs4_lat1)
			/ sizeof (from_ucs4_lat1[0])))
		      {
			if (ch >= 0x0391 && ch <= 0x0451)
			  cp = from_ucs4_greek[ch - 0x391];
			else if (ch >= 0x2010 && ch <= 0x9fa0)
			  cp = from_ucs4_cjk[ch - 0x02010];
			else
			  break;
		      }
		    else
		      cp = from_ucs4_lat1[ch];
		  */
		  johab_from_ucs4 (ch, cp);

		  if (cp[0] == '\0' && ch != 0)
		    /* Illegal character.  */
		    break;

		  outbuf[outchars] = cp[0];
		  /* Now test for a possible second byte and write this
		     if possible.  */
		  if (cp[1] != '\0')
		    {
		      if (outchars + 1 >= data->outbufsize)
			{
			  /* The result does not fit into the buffer.  */
			  extra = 1;
			  break;
			}
		      outbuf[++outchars] = cp[1];
		    }

		  ++do_write;
		  ++outchars;
		  cnt += sizeof (wchar_t);
		}
	      *inbufsize -= cnt;
	      data->outbufavail = outchars;

	      if (outchars + extra < data->outbufsize)
		{
		  /* If there is still room in the output buffer something
		     is wrong with the input.  */
		  if (inwchars >= cnt + sizeof (wchar_t))
		    {
		      /* An error occurred.  */
		      result = GCONV_ILLEGAL_INPUT;
		      break;
		    }
		  if (inwchars != cnt)
		    {
		      /* There are some unprocessed bytes at the end of the
		         input buffer.  */
		      result = GCONV_INCOMPLETE_INPUT;
		      break;
		    }
		}
	    }

	  if (result != GCONV_OK)
	    break;

	  if (data->is_last)
	    {
	      /* This is the last step.  */
	      result = (*inbufsize > (step->data == &from_johab_object
				      ? 0 : sizeof (wchar_t) - 1)
			? GCONV_FULL_OUTPUT : GCONV_EMPTY_INPUT);
	      break;
	    }

	  /* Status so far.  */
	  result = GCONV_EMPTY_INPUT;

	  if (data->outbufavail > 0)
	    {
	      /* Call the functions below in the chain.  */
	      size_t newavail = data->outbufavail;

	      result = (*fct) (next_step, next_data, data->outbuf, &newavail,
			       written, 0);

	      /* Correct the output buffer.  */
	      if (newavail != data->outbufavail && newavail > 0)
		{
		  memmove (data->outbuf,
			   &data->outbuf[data->outbufavail - newavail],
			   newavail);
		  data->outbufavail = newavail;
		}
	    }
	}
      while (*inbufsize > 0 && result == GCONV_EMPTY_INPUT);
    }

  if (written != NULL && data->is_last)
    *written = do_write;

  return result;
}
