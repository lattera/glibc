/* Conversion module for ISO-2022-CN-EXT.
   Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 2000.

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

#include <dlfcn.h>
#include <gconv.h>
#include <stdint.h>
#include <string.h>
#include "gb2312.h"
#include "iso-ir-165.h"
#include "cns11643l1.h"
#include "cns11643l2.h"

#include <assert.h>

/* This makes obvious what everybody knows: 0x1b is the Esc character.  */
#define ESC	0x1b

/* We have single-byte shift-in and shift-out sequences, and the single
   shift sequences SS2 and SS3 which replaces the SS2/SS3 designation for
   the next two bytes.  */
#define SI	0x0f
#define SO	0x0e
#define SS2_0	ESC
#define SS2_1	0x4e
#define SS3_0	ESC
#define SS3_1	0x4f

/* Definitions used in the body of the `gconv' function.  */
#define CHARSET_NAME		"ISO-2022-CN-EXT//"
#define DEFINE_INIT		1
#define DEFINE_FINI		1
#define FROM_LOOP		from_iso2022cn_ext_loop
#define TO_LOOP			to_iso2022cn_ext_loop
#define MIN_NEEDED_FROM		1
#define MAX_NEEDED_FROM		4
#define MIN_NEEDED_TO		4
#define MAX_NEEDED_TO		4
#define PREPARE_LOOP \
  int save_set;								      \
  int *setp = &data->__statep->__count;
#define EXTRA_LOOP_ARGS		, setp


/* The charsets GB/T 12345-90, GB 7589-87, GB/T 13131-9X, GB 7590-87,
   and GB/T 13132-9X are not registered to the best of my knowledge and
   therefore have no escape sequence assigned.  We cannot handle them
   for this reason.  Tell the implementation about this.  */
#define X12345	'\0'
#define X7589	'\0'
#define X13131	'\0'
#define X7590	'\0'
#define X13132	'\0'


/* The COUNT element of the state keeps track of the currently selected
   character set.  The possible values are:  */
enum
{
  ASCII_set = 0,
  GB2312_set,
  GB12345_set,
  CNS11643_1_set,
  ISO_IR_165_set,
  SO_mask = 7,

  GB7589_set = 8,
  GB13131_set = 16,
  CNS11643_2_set = 24,
  SS2_mask = 24,

  GB7590_set = 0,
  GB13132_set = 32,
  CNS11643_3_set = 64,
  CNS11643_4_set = 96,
  CNS11643_5_set = 128,
  CNS11643_6_set = 160,
  CNS11643_7_set = 192,
  SS3_mask = 224,

#define CURRENT_MASK (SO_mask | SS2_mask | SS3_mask)

  GB2312_ann = 256,
  GB12345_ann = 512,
  CNS11643_1_ann = 768,
  ISO_IR_165_ann = 1024,
  SO_ann = 1792,

  GB7589_ann = 2048,
  GB13131_ann = 4096,
  CNS11643_2_ann = 6144,
  SS2_ann = 6144,

  GB7590_ann = 8192,
  GB13132_ann = 16384,
  CNS11643_3_ann = 24576,
  CNS11643_4_ann = 32768,
  CNS11643_5_ann = 40960,
  CNS11643_6_ann = 49152,
  CNS11643_7_ann = 57344,
  SS3_ann = 57344
};


/* Since this is a stateful encoding we have to provide code which resets
   the output state to the initial state.  This has to be done during the
   flushing.  */
#define EMIT_SHIFT_TO_INIT \
  if (data->__statep->__count >> 2 != ASCII_set)			      \
    {									      \
      if (FROM_DIRECTION)						      \
	/* It's easy, we don't have to emit anything, we just reset the	      \
	   state for the input.  */					      \
	data->__statep->__count = ASCII_set << 2;			      \
      else								      \
	{								      \
	  unsigned char *outbuf = data->__outbuf;			      \
	  								      \
	  /* We are not in the initial state.  To switch back we have	      \
	     to emit `SI'.  */						      \
	  if (outbuf == data->__outbufend)				      \
	    /* We don't have enough room in the output buffer.  */	      \
	    status = __GCONV_FULL_OUTPUT;				      \
	  else								      \
	    {								      \
	      /* Write out the shift sequence.  */			      \
	      *outbuf++ = SI;						      \
	      if (data->__flags & __GCONV_IS_LAST)			      \
		*irreversible += 1;					      \
	      data->__outbuf = outbuf;					      \
	      data->__statep->__count = ASCII_set << 2;			      \
	    }								      \
	}								      \
    }


/* Since we might have to reset input pointer we must be able to save
   and retore the state.  */
#define SAVE_RESET_STATE(Save) \
  if (Save)								      \
    save_set = *setp;							      \
  else									      \
    *setp = save_set


/* First define the conversion function from ISO-2022-CN to UCS4.  */
#define MIN_NEEDED_INPUT	MIN_NEEDED_FROM
#define MAX_NEEDED_INPUT	MAX_NEEDED_FROM
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_TO
#define LOOPFCT			FROM_LOOP
#define BODY \
  {									      \
    uint32_t ch = *inptr;						      \
									      \
    /* This is a 7bit character set, disallow all 8bit characters.  */	      \
    if (ch > 0x7f)							      \
      {									      \
	if (! ignore_errors_p ())					      \
	  {								      \
	    result = __GCONV_ILLEGAL_INPUT;				      \
	    break;							      \
	  }								      \
	++inptr;							      \
	++*irreversible;						      \
	continue;							      \
      }									      \
									      \
    /* Recognize escape sequences.  */					      \
    if (ch == ESC)							      \
      {									      \
	/* There are three kinds of escape sequences we have to handle:	      \
	   - those announcing the use of GB and CNS characters on the	      \
	     line; we can simply ignore them				      \
	   - the initial byte of the SS2 sequence.			      \
	   - the initial byte of the SS3 sequence.			      \
	*/								      \
	if (inptr + 1 > inend						      \
	    || (inptr[1] == '$'						      \
		&& (inptr + 2 > inend					      \
		    || (inptr[2] == ')' && inptr + 3 > inend)		      \
		    || (inptr[2] == '*' && inptr + 3 > inend)		      \
		    || (inptr[2] == '+' && inptr + 3 > inend)))		      \
	    || (inptr[1] == SS2_1 && inptr + 3 > inend)			      \
	    || (inptr[1] == SS3_1 && inptr + 3 > inend))		      \
	  {								      \
	    result = __GCONV_EMPTY_INPUT;				      \
	    break;							      \
	  }								      \
	if (inptr[1] == '$'						      \
	    && ((inptr[2] == ')'					      \
		 && (inptr[3] == 'A'					      \
		     || (X12345 != '\0' && inptr[3] == X12345)		      \
		     || inptr[3] == 'E' || inptr[3] == 'G'))		      \
		|| (inptr[2] == '*'					      \
		    && ((X7589 != '\0' && inptr[3] == X7589)		      \
			|| (X13131 != '\0' && inptr[3] == X13131)	      \
			|| inptr[3] == 'H'))				      \
		|| (inptr[2] == '+'					      \
		    && ((X7590 != '\0' && inptr[3] == X7590)		      \
			|| (X13132 != '\0' && inptr[3] == X13132)	      \
			|| inptr[3] == 'I' || inptr[3] == 'J'		      \
			|| inptr[3] == 'K' || inptr[3] == 'L'		      \
			|| inptr[3] == 'M'))))				      \
	  {								      \
	    /* OK, we accept those character sets.  */			      \
	    if (inptr[3] == 'A')					      \
	      ann = (ann & ~SO_ann) | GB2312_ann;			      \
	    else if (inptr[3] == 'G')					      \
	      ann = (ann & ~SO_ann) | CNS11643_1_ann;			      \
	    else if (inptr[3] == 'E')					      \
	      ann = (ann & ~SO_ann) | ISO_IR_165_ann;			      \
	    else if (X12345 != '\0' && inptr[3] == X12345)		      \
	      ann = (ann & ~SO_ann) | GB12345_ann;			      \
	    else if (inptr[3] == 'H')					      \
	      ann = (ann & ~SS2_ann) | CNS11643_2_ann;			      \
	    else if (X7589 != '\0' && inptr[3] == X7589)		      \
	      ann = (ann & ~SS2_ann) | GB7589_ann;			      \
	    else if (X13131 != '\0' && inptr[3] == X13131)		      \
	      ann = (ann & ~SS2_ann) | GB13131_ann;			      \
	    else if (inptr[3] == 'I')					      \
	      ann = (ann & ~SS3_ann) | CNS11643_3_ann;			      \
	    else if (inptr[3] == 'J')					      \
	      ann = (ann & ~SS3_ann) | CNS11643_4_ann;			      \
	    else if (inptr[3] == 'K')					      \
	      ann = (ann & ~SS3_ann) | CNS11643_5_ann;			      \
	    else if (inptr[3] == 'L')					      \
	      ann = (ann & ~SS3_ann) | CNS11643_6_ann;			      \
	    else if (inptr[3] == 'M')					      \
	      ann = (ann & ~SS3_ann) | CNS11643_7_ann;			      \
	    else if (X7590 != '\0' && inptr[3] == X7590)		      \
	      ann = (ann & ~SS3_ann) | GB7590_ann;			      \
	    else if (X13132 != '\0' && inptr[3] == X13132)		      \
	      ann = (ann & ~SS3_ann) | GB13132_ann;			      \
	    inptr += 4;							      \
	    continue;							      \
	  }								      \
      }									      \
    else if (ch == SO)							      \
      {									      \
	/* Switch to use GB2312, GB12345, CNS 11643 plane 1, or ISO-IR-165,   \
	   depending on which S0 designation came last.  The only problem     \
	   is what to do with faulty input files where no designator came.    \
	   XXX For now I'll default to use GB2312.  If this is not the	      \
	   best behavior (e.g., we should flag an error) let me know.  */     \
	++inptr;							      \
	switch (ann & SO_ann)						      \
	  {								      \
	  case GB2312_ann:						      \
	    set = GB2312_set;						      \
	    break;							      \
	  case GB12345_ann:						      \
	    set = GB12345_set;						      \
	    break;							      \
	  case CNS11643_1_ann:						      \
	    set = CNS11643_1_set;					      \
	    break;							      \
	  default:							      \
	    assert ((ann & SO_ann) == ISO_IR_165_ann);			      \
	    set = ISO_IR_165_set;					      \
	    break;							      \
	  }								      \
	continue;							      \
      }									      \
    else if (ch == SI)							      \
      {									      \
	/* Switch to use ASCII.  */					      \
	++inptr;							      \
	set = ASCII_set;						      \
	continue;							      \
      }									      \
									      \
    if (ch == ESC && (inend - inptr == 1 || inptr[1] == SS2_1))		      \
      {									      \
	/* This is a character from CNS 11643 plane 2.			      \
	   XXX We could test here whether the use of this character	      \
	   set was announced.						      \
	   XXX Current GB7589 and GB13131 are not supported.  */	      \
	if (inend - inptr < 4)						      \
	  {								      \
	    result = __GCONV_INCOMPLETE_INPUT;				      \
	    break;							      \
	  }								      \
	inptr += 2;							      \
	ch = cns11643l2_to_ucs4 (&inptr, 2, 0);				      \
	if (ch == __UNKNOWN_10646_CHAR)					      \
	  {								      \
	    if (! ignore_errors_p ())					      \
	      {								      \
		inptr -= 2;						      \
		result = __GCONV_ILLEGAL_INPUT;				      \
		break;							      \
	      }								      \
	    ++*irreversible;						      \
	    continue;							      \
	  }								      \
      }									      \
    /* Note that we can assume here that at least bytes are available if      \
       the first byte is ESC since otherwise the first if would have been     \
       true.  */							      \
    else if (ch == ESC && inptr[1] == SS3_1)				      \
      {									      \
	/* This is a character from CNS 11643 plane 3 or higher.	      \
	   XXX Current GB7590 and GB13132 are not supported.  */	      \
	if (inend - inptr < 4)						      \
	  {								      \
	    result = __GCONV_INCOMPLETE_INPUT;				      \
	    break;							      \
	  }								      \
	inptr += 2;							      \
	ch = cns11643l2_to_ucs4 (&inptr, 2, 0);				      \
	if (ch == __UNKNOWN_10646_CHAR)					      \
	  {								      \
	    if (! ignore_errors_p ())					      \
	      {								      \
		inptr -= 2;						      \
		result = __GCONV_ILLEGAL_INPUT;				      \
		break;							      \
	      }								      \
	    ++*irreversible;						      \
	    continue;							      \
	  }								      \
      }									      \
    else if (set == ASCII_set)						      \
      {									      \
	/* Almost done, just advance the input pointer.  */		      \
	++inptr;							      \
      }									      \
    else								      \
      {									      \
	/* That's pretty easy, we have a dedicated functions for this.  */    \
	if (inend - inptr < 2)						      \
	  {								      \
	    result = __GCONV_INCOMPLETE_INPUT;				      \
	    break;							      \
	  }								      \
	if (set == GB2312_set)						      \
	  ch = gb2312_to_ucs4 (&inptr, inend - inptr, 0);		      \
	else if (set == ISO_IR_165_set)					      \
	  ch = isoir165_to_ucs4 (&inptr, inend - inptr);		      \
	else								      \
	  {								      \
	    assert (set == CNS11643_1_set);				      \
	    ch = cns11643l1_to_ucs4 (&inptr, inend - inptr, 0);		      \
	  }								      \
									      \
	if (ch == 0)							      \
	  {								      \
	    result = __GCONV_EMPTY_INPUT;				      \
	    break;							      \
	  }								      \
	else if (ch == __UNKNOWN_10646_CHAR)				      \
	  {								      \
	    if (! ignore_errors_p ())					      \
	      {								      \
		result = __GCONV_ILLEGAL_INPUT;				      \
		break;							      \
	      }								      \
	    inptr += 2;							      \
	    ++*irreversible;						      \
	    continue;							      \
	  }								      \
      }									      \
									      \
    *((uint32_t *) outptr)++ = ch;					      \
  }
#define EXTRA_LOOP_DECLS	, int *setp
#define INIT_PARAMS		int set = (*setp >> 2) & CURRENT_MASK; \
				int ann = (*setp >> 2) & ~CURRENT_MASK
#define UPDATE_PARAMS		*setp = (set | ann) << 2
#define LOOP_NEED_FLAGS
#include <iconv/loop.c>


/* Next, define the other direction.  */
#define MIN_NEEDED_INPUT	MIN_NEEDED_TO
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_FROM
#define MAX_NEEDED_OUTPUT	MAX_NEEDED_FROM
#define LOOPFCT			TO_LOOP
#define BODY \
  {									      \
    uint32_t ch;							      \
    size_t written = 0;							      \
									      \
    ch = *((uint32_t *) inptr);						      \
									      \
    /* First see whether we can write the character using the currently	      \
       selected character set.  */					      \
    if (ch < 0x80)							      \
      {									      \
	if (set != ASCII_set)						      \
	  {								      \
	    *outptr++ = SI;						      \
	    set = ASCII_set;						      \
	    if (outptr == outend)					      \
	      {								      \
		result = __GCONV_FULL_OUTPUT;				      \
		break;							      \
	      }								      \
	  }								      \
									      \
	*outptr++ = ch;							      \
	written = 1;							      \
									      \
	/* At the end of the line we have to clear the `ann' flags since      \
	   every line must contain this information again.  */		      \
	if (ch == L'\n')						      \
	  ann = 0;							      \
      }									      \
    else								      \
      {									      \
	char buf[2];							      \
	int used;							      \
									      \
	if (set == GB2312_set || ((ann & CNS11643_1_ann) == 0		      \
				  && (ann & ISO_IR_165_ann) == 0))	      \
	  {								      \
	    written = ucs4_to_gb2312 (ch, buf, 2);			      \
	    used = GB2312_set;						      \
	  }								      \
	else if (set == ISO_IR_165_set || (ann & ISO_IR_165_set) != 0)	      \
	  {								      \
	    written = ucs4_to_gb2312 (ch, buf, 2);			      \
	    used = GB2312_set;						      \
	  }								      \
	else								      \
	  {								      \
	    written = ucs4_to_cns11643l1 (ch, buf, 2);			      \
	    used = CNS11643_1_set;					      \
	  }								      \
									      \
	if (written == __UNKNOWN_10646_CHAR)				      \
	  {								      \
	    /* Cannot convert it using the currently selected SO set.	      \
	       Next try the SS2 set.  */				      \
	    written = ucs4_to_cns11643l2 (ch, buf, 2);			      \
	    if (written != __UNKNOWN_10646_CHAR)			      \
	      /* Yep, that worked.  */					      \
	      used = CNS11643_2_set;					      \
	    else							      \
	      {								      \
		/* Well, see whether we have to change the SO set.  */	      \
		if (set != GB2312_set)					      \
		  {							      \
		    written = ucs4_to_gb2312 (ch, buf, 2);		      \
		    if (written != __UNKNOWN_10646_CHAR)		      \
		      used = GB2312_set;				      \
		  }							      \
		if (written == __UNKNOWN_10646_CHAR && set != ISO_IR_165_set) \
		  {							      \
		    written = ucs4_to_isoir165 (ch, buf, 2);		      \
		    if (written != __UNKNOWN_10646_CHAR)		      \
		      used = ISO_IR_165_set;				      \
		  }							      \
		if (written == __UNKNOWN_10646_CHAR && set != CNS11643_1_set) \
		  {							      \
		    written = ucs4_to_cns11643l1 (ch, buf, 2);		      \
		    if (written != __UNKNOWN_10646_CHAR)		      \
		      used = CNS11643_1_set;				      \
		  }							      \
									      \
		if (written == __UNKNOWN_10646_CHAR)			      \
		  {							      \
		    /* Even this does not work.  Error.  */		      \
		    STANDARD_ERR_HANDLER (4);				      \
		  }							      \
	      }								      \
	  }								      \
	assert (written == 2);						      \
									      \
	/* See whether we have to emit an escape sequence.  */		      \
	if (set != used)						      \
	  {								      \
	    /* First see whether we announced that we use this		      \
	       character set.  */					      \
	    if ((ann & (2 << used)) == 0)				      \
	      {								      \
		const char *escseq;					      \
									      \
		if (outptr + 4 > outend)				      \
		  {							      \
		    result = __GCONV_FULL_OUTPUT;			      \
		    break;						      \
		  }							      \
									      \
		assert (used >= 1 && used <= 4);			      \
		escseq = "\e$)A\e$)G\e$*H\e$)E" + (used - 1) * 4;	      \
		*outptr++ = *escseq++;					      \
		*outptr++ = *escseq++;					      \
		*outptr++ = *escseq++;					      \
		*outptr++ = *escseq++;					      \
									      \
		if (used == GB2312_set)					      \
		  ann = (ann & CNS11643_2_ann) | GB2312_ann;		      \
		else if (used == CNS11643_1_set)			      \
		  ann = (ann & CNS11643_2_ann) | CNS11643_1_ann;	      \
		else							      \
		  ann |= CNS11643_2_ann;				      \
	      }								      \
									      \
	    if (used == CNS11643_2_set)					      \
	      {								      \
		if (outptr + 2 > outend)				      \
		  {							      \
		    result = __GCONV_FULL_OUTPUT;			      \
		    break;						      \
		  }							      \
		*outptr++ = SS2_0;					      \
		*outptr++ = SS2_1;					      \
	      }								      \
	    else							      \
	      {								      \
		/* We only have to emit something if currently ASCII is	      \
		   selected.  Otherwise we are switching within the	      \
		   SO charset.  */					      \
		if (set == ASCII_set)					      \
		  {							      \
		    if (outptr + 1 > outend)				      \
		      {							      \
			result = __GCONV_FULL_OUTPUT;			      \
			break;						      \
		      }							      \
		    *outptr++ = SO;					      \
		  }							      \
	      }								      \
									      \
	    /* Always test the length here since we have used up all the      \
	       guaranteed output buffer slots.  */			      \
	    if (outptr + 2 > outend)					      \
	      {								      \
		result = __GCONV_FULL_OUTPUT;				      \
		break;							      \
	      }								      \
	  }								      \
	else if (outptr + 2 > outend)					      \
	  {								      \
	    result = __GCONV_FULL_OUTPUT;				      \
	    break;							      \
	  }								      \
									      \
	*outptr++ = buf[0];						      \
	*outptr++ = buf[1];						      \
      }									      \
									      \
    /* Now that we wrote the output increment the input pointer.  */	      \
    inptr += 4;								      \
  }
#define EXTRA_LOOP_DECLS	, int *setp
#define INIT_PARAMS		int set = (*setp >> 2) & CURRENT_MASK; \
				int ann = (*setp >> 2) & ~CURRENT_MASK
#define UPDATE_PARAMS		*setp = (set | ann) << 2
#define LOOP_NEED_FLAGS
#include <iconv/loop.c>


/* Now define the toplevel functions.  */
#include <iconv/skeleton.c>
