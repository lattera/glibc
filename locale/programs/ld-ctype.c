/* Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <alloca.h>
#include <endian.h>
#include <limits.h>
#include <string.h>

#include "locales.h"
#include "localeinfo.h"
#include "langinfo.h"
#include "locfile-token.h"
#include "stringtrans.h"

/* Uncomment the following line in the production version.  */
/* define NDEBUG 1 */
#include <assert.h>


void *xmalloc (size_t __n);
void *xcalloc (size_t __n, size_t __s);
void *xrealloc (void *__ptr, size_t __n);


/* The bit used for representing a special class.  */
#define BITPOS(class) ((class) - tok_upper)
#define BIT(class) (1 << BITPOS (class))

#define ELEM(ctype, collection, idx, value)				      \
  *find_idx (ctype, &ctype->collection idx, &ctype->collection##_max idx,     \
	     &ctype->collection##_act idx, value)

#define SWAPU32(w) \
  (((w) << 24) | (((w) & 0xff00) << 8) | (((w) >> 8) & 0xff00) | ((w) >> 24))

#define SWAPU16(w) \
  ((((w)  >> 8) & 0xff) | (((w) & 0xff) << 8))


/* To be compatible with former implementations we for now restrict
   the number of bits for character classes to 16.  When compatibility
   is not necessary anymore increase the number to 32.  */
#define char_class_t u_int16_t
#define CHAR_CLASS_TRANS SWAPU16
#define char_class32_t u_int32_t
#define CHAR_CLASS32_TRANS SWAPU32


/* The real definition of the struct for the LC_CTYPE locale.  */
struct locale_ctype_t
{
  unsigned int *charnames;
  size_t charnames_max;
  size_t charnames_act;

  /* We will allow up to 8 * sizeof(u_int32_t) - 1 character classes.  */
#define MAX_NR_CHARCLASS (8 * sizeof (u_int32_t) - 1)
  size_t nr_charclass;
  const char *classnames[MAX_NR_CHARCLASS];
  unsigned long int current_class_mask;
  unsigned int last_class_char;
  u_int32_t *class_collection;
  size_t class_collection_max;
  size_t class_collection_act;
  unsigned long int class_done;

  /* If the following number ever turns out to be too small simply
     increase it.  But I doubt it will.  --drepper@gnu */
#define MAX_NR_CHARMAP 16
  const char *mapnames[MAX_NR_CHARMAP];
  u_int32_t *map_collection[MAX_NR_CHARMAP];
  size_t map_collection_max[MAX_NR_CHARMAP];
  size_t map_collection_act[MAX_NR_CHARMAP];
  size_t map_collection_nr;
  size_t last_map_idx;
  unsigned int from_map_char;
  int toupper_done;
  int tolower_done;

  /* The arrays for the binary representation.  */
  u_int32_t plane_size;
  u_int32_t plane_cnt;
  char_class_t *ctype_b;
  char_class32_t *ctype32_b;
  u_int32_t *names_el;
  u_int32_t *names_eb;
  u_int32_t **map_eb;
  u_int32_t **map_el;
  u_int32_t *class_name_ptr;
  u_int32_t *map_name_ptr;
  unsigned char *width;
  u_int32_t mb_cur_max;
  const char *codeset_name;
};


/* Prototypes for local functions.  */
static void ctype_class_newP (struct linereader *lr,
			      struct locale_ctype_t *ctype, const char *name);
static void ctype_map_newP (struct linereader *lr,
			    struct locale_ctype_t *ctype,
			    const char *name, struct charset_t *charset);
static u_int32_t *find_idx (struct locale_ctype_t *ctype, u_int32_t **table,
			    size_t *max, size_t *act, unsigned int idx);
static void set_class_defaults (struct locale_ctype_t *ctype,
				struct charset_t *charset);
static void allocate_arrays (struct locale_ctype_t *ctype,
			     struct charset_t *charset);


void
ctype_startup (struct linereader *lr, struct localedef_t *locale,
	       struct charset_t *charset)
{
  unsigned int cnt;
  struct locale_ctype_t *ctype;

  /* It is important that we always use UCS1 encoding for strings now.  */
  encoding_method = ENC_UCS1;

  /* Allocate the needed room.  */
  locale->categories[LC_CTYPE].ctype = ctype =
    (struct locale_ctype_t *) xmalloc (sizeof (struct locale_ctype_t));

  /* We have no names seen yet.  */
  ctype->charnames_max = charset->mb_cur_max == 1 ? 256 : 512;
  ctype->charnames =
    (unsigned int *) xmalloc (ctype->charnames_max * sizeof (unsigned int));
  for (cnt = 0; cnt < 256; ++cnt)
    ctype->charnames[cnt] = cnt;
  ctype->charnames_act = 256;

  /* Fill character class information.  */
  ctype->nr_charclass = 0;
  ctype->current_class_mask = 0;
  ctype->last_class_char = ILLEGAL_CHAR_VALUE;
  /* The order of the following instructions determines the bit
     positions!  */
  ctype_class_newP (lr, ctype, "upper");
  ctype_class_newP (lr, ctype, "lower");
  ctype_class_newP (lr, ctype, "alpha");
  ctype_class_newP (lr, ctype, "digit");
  ctype_class_newP (lr, ctype, "xdigit");
  ctype_class_newP (lr, ctype, "space");
  ctype_class_newP (lr, ctype, "print");
  ctype_class_newP (lr, ctype, "graph");
  ctype_class_newP (lr, ctype, "blank");
  ctype_class_newP (lr, ctype, "cntrl");
  ctype_class_newP (lr, ctype, "punct");
  ctype_class_newP (lr, ctype, "alnum");

  ctype->class_collection_max = charset->mb_cur_max == 1 ? 256 : 512;
  ctype->class_collection
    = (u_int32_t *) xmalloc (sizeof (unsigned long int)
			     * ctype->class_collection_max);
  memset (ctype->class_collection, '\0',
	  sizeof (unsigned long int) * ctype->class_collection_max);
  ctype->class_collection_act = 256;

  /* Fill character map information.  */
  ctype->map_collection_nr = 0;
  ctype->last_map_idx = MAX_NR_CHARMAP;
  ctype->from_map_char = ILLEGAL_CHAR_VALUE;
  ctype_map_newP (lr, ctype, "toupper", charset);
  ctype_map_newP (lr, ctype, "tolower", charset);

  /* Fill first 256 entries in `toupper' and `tolower' arrays.  */
  for (cnt = 0; cnt < 256; ++cnt)
    {
      ctype->map_collection[0][cnt] = cnt;
      ctype->map_collection[1][cnt] = cnt;
    }
}


void
ctype_finish (struct localedef_t *locale, struct charset_t *charset)
{
  /* See POSIX.2, table 2-6 for the meaning of the following table.  */
#define NCLASS 12
  static const struct
  {
    const char *name;
    const char allow[NCLASS];
  }
  valid_table[NCLASS] =
  {
    /* The order is important.  See token.h for more information.
       M = Always, D = Default, - = Permitted, X = Mutually exclusive  */
    { "upper",  "--MX-XDDXXX-" },
    { "lower",  "--MX-XDDXXX-" },
    { "alpha",  "---X-XDDXXX-" },
    { "digit",  "XXX--XDDXXX-" },
    { "xdigit", "-----XDDXXX-" },
    { "space",  "XXXXX------X" },
    { "print",  "---------X--" },
    { "graph",  "---------X--" },
    { "blank",  "XXXXXM-----X" },
    { "cntrl",  "XXXXX-XX--XX" },
    { "punct",  "XXXXX-DD-X-X" },
    { "alnum",  "-----XDDXXX-" }
  };
  size_t cnt;
  int cls1, cls2;
  unsigned int space_value;
  struct locale_ctype_t *ctype = locale->categories[LC_CTYPE].ctype;

  /* Set default value for classes not specified.  */
  set_class_defaults (ctype, charset);

  /* Check according to table.  */
  for (cnt = 0; cnt < ctype->class_collection_max; ++cnt)
    {
      unsigned long int tmp;

      tmp = ctype->class_collection[cnt];
      if (tmp == 0)
	continue;

      for (cls1 = 0; cls1 < NCLASS; ++cls1)
	if ((tmp & (1 << cls1)) != 0)
	  for (cls2 = 0; cls2 < NCLASS; ++cls2)
	    if (valid_table[cls1].allow[cls2] != '-')
	      {
		int eq = (tmp & (1 << cls2)) != 0;
		switch (valid_table[cls1].allow[cls2])
		  {
		  case 'M':
		    if (!eq)
		      {
			char buf[17];
			char *cp = buf;
			unsigned int value;

			value = ctype->charnames[cnt];

			if ((value & 0xff000000) != 0)
			  cp += sprintf (cp, "\\%o", (value >> 24) & 0xff);
			if ((value & 0xffff0000) != 0)
			  cp += sprintf (cp, "\\%o", (value >> 16) & 0xff);
			if ((value & 0xffffff00) != 0)
			  cp += sprintf (cp, "\\%o", (value >> 8) & 0xff);
			sprintf (cp, "\\%o", value & 0xff);

			if (!be_quiet)
			  error (0, 0, _("\
character %s'%s' in class `%s' must be in class `%s'"), value > 256 ? "L" : "",
				 cp, valid_table[cls1].name,
				 valid_table[cls2].name);
		      }
		    break;

		  case 'X':
		    if (eq)
		      {
			char buf[17];
			char *cp = buf;
			unsigned int value;

			value = ctype->charnames[cnt];

			if ((value & 0xff000000) != 0)
			  cp += sprintf (cp, "\\%o", value >> 24);
			if ((value & 0xffff0000) != 0)
			  cp += sprintf (cp, "\\%o", (value >> 16) & 0xff);
			if ((value & 0xffffff00) != 0)
			  cp += sprintf (cp, "\\%o", (value >> 8) & 0xff);
			sprintf (cp, "\\%o", value & 0xff);

			if (!be_quiet)
			  error (0, 0, _("\
character %s'%s' in class `%s' must not be in class `%s'"),
				 value > 256 ? "L" : "", cp,
				 valid_table[cls1].name,
				 valid_table[cls2].name);
		      }
		    break;

		  case 'D':
		    ctype->class_collection[cnt] |= 1 << cls2;
		    break;

		  default:
		    error (5, 0, _("internal error in %s, line %u"),
			   __FUNCTION__, __LINE__);
                  }
              }
    }

  /* ... and now test <SP> as a special case.  */
  space_value = charset_find_value (&charset->char_table, "SP", 2);
  if ((wchar_t) space_value == ILLEGAL_CHAR_VALUE)
    {
      if (!be_quiet)
	error (0, 0, _("character <SP> not defined in character map"));
    }
  else if (((cnt = BITPOS (tok_space),
	     (ELEM (ctype, class_collection, , space_value)
	      & BIT (tok_space)) == 0)
	    || (cnt = BITPOS (tok_blank),
		(ELEM (ctype, class_collection, , space_value)
		 & BIT (tok_blank)) == 0)))
    {
      if (!be_quiet)
	error (0, 0, _("<SP> character not in class `%s'"),
	       valid_table[cnt].name);
    }
  else if (((cnt = BITPOS (tok_punct),
	     (ELEM (ctype, class_collection, , space_value)
	      & BIT (tok_punct)) != 0)
	    || (cnt = BITPOS (tok_graph),
		(ELEM (ctype, class_collection, , space_value)
		 & BIT (tok_graph))
		!= 0)))
    {
      if (!be_quiet)
	error (0, 0, _("<SP> character must not be in class `%s'"),
	       valid_table[cnt].name);
    }
  else
    ELEM (ctype, class_collection, , space_value) |= BIT (tok_print);

  /* Now that the tests are done make sure the name array contains all
     characters which are handled in the WIDTH section of the
     character set definition file.  */
  if (charset->width_rules != NULL)
    for (cnt = 0; cnt < charset->nwidth_rules; ++cnt)
      {
	size_t inner;
	for (inner = charset->width_rules[cnt].from;
	     inner <= charset->width_rules[cnt].to; ++inner)
	  (void) find_idx (ctype, NULL, NULL, NULL, inner);
      }
}


void
ctype_output (struct localedef_t *locale, struct charset_t *charset,
	      const char *output_path)
{
  struct locale_ctype_t *ctype = locale->categories[LC_CTYPE].ctype;
  const size_t nelems = (_NL_ITEM_INDEX (_NL_NUM_LC_CTYPE)
			 + 2 * (ctype->map_collection_nr - 2));
  struct iovec iov[2 + nelems + ctype->nr_charclass
		  + ctype->map_collection_nr];
  struct locale_file data;
  u_int32_t idx[nelems];
  size_t elem, cnt, offset, total;


  if ((locale->binary & (1 << LC_CTYPE)) != 0)
    {
      iov[0].iov_base = ctype;
      iov[0].iov_len = locale->len[LC_CTYPE];

      write_locale_data (output_path, "LC_CTYPE", 1, iov);

      return;
    }


  /* Now prepare the output: Find the sizes of the table we can use.  */
  allocate_arrays (ctype, charset);

  data.magic = LIMAGIC (LC_CTYPE);
  data.n = nelems;
  iov[0].iov_base = (void *) &data;
  iov[0].iov_len = sizeof (data);

  iov[1].iov_base = (void *) idx;
  iov[1].iov_len = sizeof (idx);

  idx[0] = iov[0].iov_len + iov[1].iov_len;
  offset = 0;

  for (elem = 0; elem < nelems; ++elem)
    {
      if (elem < _NL_ITEM_INDEX (_NL_NUM_LC_CTYPE))
	switch (elem)
	  {
#define CTYPE_DATA(name, base, len)					      \
	  case _NL_ITEM_INDEX (name):					      \
	    iov[2 + elem + offset].iov_base = (base);			      \
	    iov[2 + elem + offset].iov_len = (len);			      \
	    if (elem + 1 < nelems)					      \
	      idx[elem + 1] = idx[elem] + iov[2 + elem + offset].iov_len;     \
	    break

	  CTYPE_DATA (_NL_CTYPE_CLASS,
		      ctype->ctype_b,
		      (256 + 128) * sizeof (char_class_t));

	  CTYPE_DATA (_NL_CTYPE_TOUPPER_EB,
		      ctype->map_eb[0],
		      (ctype->plane_size * ctype->plane_cnt + 128)
		      * sizeof (u_int32_t));
	  CTYPE_DATA (_NL_CTYPE_TOLOWER_EB,
		      ctype->map_eb[1],
		      (ctype->plane_size * ctype->plane_cnt + 128)
		      * sizeof (u_int32_t));

	  CTYPE_DATA (_NL_CTYPE_TOUPPER_EL,
		      ctype->map_el[0],
		      (ctype->plane_size * ctype->plane_cnt + 128)
		      * sizeof (u_int32_t));
	  CTYPE_DATA (_NL_CTYPE_TOLOWER_EL,
		      ctype->map_el[1],
		      (ctype->plane_size * ctype->plane_cnt + 128)
		      * sizeof (u_int32_t));

	  CTYPE_DATA (_NL_CTYPE_CLASS32,
		      ctype->ctype32_b,
		      (ctype->plane_size * ctype->plane_cnt
		       * sizeof (char_class32_t)));

	  CTYPE_DATA (_NL_CTYPE_NAMES_EB,
		      ctype->names_eb, (ctype->plane_size * ctype->plane_cnt
					* sizeof (u_int32_t)));
	  CTYPE_DATA (_NL_CTYPE_NAMES_EL,
		      ctype->names_el, (ctype->plane_size * ctype->plane_cnt
					* sizeof (u_int32_t)));

	  CTYPE_DATA (_NL_CTYPE_HASH_SIZE,
		      &ctype->plane_size, sizeof (u_int32_t));
	  CTYPE_DATA (_NL_CTYPE_HASH_LAYERS,
		      &ctype->plane_cnt, sizeof (u_int32_t));

	  case _NL_ITEM_INDEX (_NL_CTYPE_CLASS_NAMES):
	    /* The class name array.  */
	    total = 0;
	    for (cnt = 0; cnt < ctype->nr_charclass; ++cnt, ++offset)
	      {
		iov[2 + elem + offset].iov_base
		  = (void *) ctype->classnames[cnt];
		iov[2 + elem + offset].iov_len
		  = strlen (ctype->classnames[cnt]) + 1;
		total += iov[2 + elem + offset].iov_len;
	      }
	    iov[2 + elem + offset].iov_base = (void *) "\0\0\0";
	    iov[2 + elem + offset].iov_len = 1 + (4 - ((total + 1) % 4));
	    total += 1 + (4 - ((total + 1) % 4));

	    if (elem + 1 < nelems)
	      idx[elem + 1] = idx[elem] + total;
	    break;

	  case _NL_ITEM_INDEX (_NL_CTYPE_MAP_NAMES):
	    /* The class name array.  */
	    total = 0;
	    for (cnt = 0; cnt < ctype->map_collection_nr; ++cnt, ++offset)
	      {
		iov[2 + elem + offset].iov_base
		  = (void *) ctype->mapnames[cnt];
		iov[2 + elem + offset].iov_len
		  = strlen (ctype->mapnames[cnt]) + 1;
		total += iov[2 + elem + offset].iov_len;
	      }
	    iov[2 + elem + offset].iov_base = (void *) "\0\0\0";
	    iov[2 + elem + offset].iov_len = 1 + (4 - ((total + 1) % 4));
	    total += 1 + (4 - ((total + 1) % 4));

	    if (elem + 1 < nelems)
	      idx[elem + 1] = idx[elem] + total;
	    break;

	  CTYPE_DATA (_NL_CTYPE_WIDTH,
		      ctype->width, ctype->plane_size * ctype->plane_cnt);

	  CTYPE_DATA (_NL_CTYPE_MB_CUR_MAX,
		      &ctype->mb_cur_max, sizeof (u_int32_t));

	  case _NL_ITEM_INDEX (_NL_CTYPE_CODESET_NAME):
	    total = strlen (ctype->codeset_name) + 1;
	    if (total % 4 == 0)
	      iov[2 + elem + offset].iov_base = (char *) ctype->codeset_name;
	    else
	      {
		iov[2 + elem + offset].iov_base = alloca ((total + 3) & ~3);
		memset (mempcpy (iov[2 + elem + offset].iov_base,
				 ctype->codeset_name, total),
			'\0', 4 - (total & 3));
		total = (total + 3) & ~3;
	      }
	    iov[2 + elem + offset].iov_len = total;
	    if (elem + 1 < nelems)
	      idx[elem + 1] = idx[elem] + iov[2 + elem + offset].iov_len;
	    break;

	  default:
	    assert (! "unknown CTYPE element");
	  }
      else
	{
	  /* Handle extra maps.  */
	  size_t nr = (elem - _NL_ITEM_INDEX (_NL_NUM_LC_CTYPE)) >> 1;

	  if (((elem - _NL_ITEM_INDEX (_NL_NUM_LC_CTYPE)) & 1) == 0)
	    iov[2 + elem + offset].iov_base = ctype->map_eb[nr];
	  else
	    iov[2 + elem + offset].iov_base = ctype->map_el[nr];

	  iov[2 + elem + offset].iov_len = ((ctype->plane_size
					     * ctype->plane_cnt + 128)
					    * sizeof (u_int32_t));

	  if (elem + 1 < nelems)
	    idx[elem + 1] = idx[elem] + iov[2 + elem + offset].iov_len;
	}
    }

  assert (2 + elem + offset == (nelems + ctype->nr_charclass
				+ ctype->map_collection_nr + 2));

  write_locale_data (output_path, "LC_CTYPE", 2 + elem + offset, iov);
}


/* Character class handling.  */
void
ctype_class_new (struct linereader *lr, struct localedef_t *locale,
		 enum token_t tok, struct token *code,
		 struct charset_t *charset)
{
  ctype_class_newP (lr, locale->categories[LC_CTYPE].ctype,
		    code->val.str.start);
}


int
ctype_is_charclass (struct linereader *lr, struct localedef_t *locale,
		    const char *name)
{
  size_t cnt;

  for (cnt = 0; cnt < locale->categories[LC_CTYPE].ctype->nr_charclass; ++cnt)
    if (strcmp (name, locale->categories[LC_CTYPE].ctype->classnames[cnt])
	== 0)
      return 1;

  return 0;
}


void
ctype_class_start (struct linereader *lr, struct localedef_t *locale,
		   enum token_t tok, const char *str,
		   struct charset_t *charset)
{
  struct locale_ctype_t *ctype = locale->categories[LC_CTYPE].ctype;
  size_t cnt;

  switch (tok)
    {
    case tok_upper:
      str = "upper";
      break;
    case tok_lower:
      str = "lower";
      break;
    case tok_alpha:
      str = "alpha";
      break;
    case tok_digit:
      str = "digit";
      break;
    case tok_xdigit:
      str = "xdigit";
      break;
    case tok_space:
      str = "space";
      break;
    case tok_print:
      str = "print";
      break;
    case tok_graph:
      str = "graph";
      break;
    case tok_blank:
      str = "blank";
      break;
    case tok_cntrl:
      str = "cntrl";
      break;
    case tok_punct:
      str = "punct";
      break;
    case tok_alnum:
      str = "alnum";
      break;
    case tok_ident:
      break;
    default:
      assert (! "illegal token as class name: should not happen");
    }

  for (cnt = 0; cnt < ctype->nr_charclass; ++cnt)
    if (strcmp (str, ctype->classnames[cnt]) == 0)
      break;

  if (cnt >= ctype->nr_charclass)
    assert (! "unknown class in class definition: should not happen");

  ctype->class_done |= BIT (tok);

  ctype->current_class_mask = 1 << cnt;
  ctype->last_class_char = ILLEGAL_CHAR_VALUE;
}


void
ctype_class_from (struct linereader *lr, struct localedef_t *locale,
		  struct token *code, struct charset_t *charset)
{
  struct locale_ctype_t *ctype = locale->categories[LC_CTYPE].ctype;
  unsigned int value;

  value = charset_find_value (&charset->char_table, code->val.str.start,
			      code->val.str.len);

  ctype->last_class_char = value;

  if ((wchar_t) value == ILLEGAL_CHAR_VALUE)
    /* In the LC_CTYPE category it is no error when a character is
       not found.  This has to be ignored silently.  */
    return;

  *find_idx (ctype, &ctype->class_collection, &ctype->class_collection_max,
	     &ctype->class_collection_act, value)
    |= ctype->current_class_mask;
}


void
ctype_class_to (struct linereader *lr, struct localedef_t *locale,
		struct token *code, struct charset_t *charset)
{
  struct locale_ctype_t *ctype = locale->categories[LC_CTYPE].ctype;
  unsigned int value, cnt;

  value = charset_find_value (&charset->char_table, code->val.str.start,
			      code->val.str.len);

  /* In the LC_CTYPE category it is no error when a character is
     not found.  This has to be ignored silently.  */
  if ((wchar_t) ctype->last_class_char != ILLEGAL_CHAR_VALUE
      && (wchar_t) value != ILLEGAL_CHAR_VALUE)
    for (cnt = ctype->last_class_char + 1; cnt <= value; ++cnt)
      *find_idx (ctype, &ctype->class_collection, &ctype->class_collection_max,
		 &ctype->class_collection_act, cnt)
	|= ctype->current_class_mask;

  ctype->last_class_char = ILLEGAL_CHAR_VALUE;
}


void
ctype_class_end (struct linereader *lr, struct localedef_t *locale)
{
  struct locale_ctype_t *ctype = locale->categories[LC_CTYPE].ctype;

  /* We have no special actions to perform here.  */
  ctype->current_class_mask = 0;
  ctype->last_class_char = ILLEGAL_CHAR_VALUE;
}


/* Character map handling.  */
void
ctype_map_new (struct linereader *lr, struct localedef_t *locale,
	       enum token_t tok, struct token *code,
	       struct charset_t *charset)
{
  ctype_map_newP (lr, locale->categories[LC_CTYPE].ctype,
		  code->val.str.start, charset);
}


int
ctype_is_charconv (struct linereader *lr, struct localedef_t *locale,
		   const char *name)
{
  struct locale_ctype_t *ctype = locale->categories[LC_CTYPE].ctype;
  size_t cnt;

  for (cnt = 0; cnt < ctype->map_collection_nr; ++cnt)
    if (strcmp (name, ctype->mapnames[cnt]) == 0)
      return 1;

  return 0;
}


void
ctype_map_start (struct linereader *lr, struct localedef_t *locale,
		 enum token_t tok, const char *name, struct charset_t *charset)
{
  struct locale_ctype_t *ctype = locale->categories[LC_CTYPE].ctype;
  size_t cnt;

  switch (tok)
    {
    case tok_toupper:
      ctype->toupper_done = 1;
      name = "toupper";
      break;
    case tok_tolower:
      ctype->tolower_done = 1;
      name = "tolower";
      break;
    case tok_ident:
      break;
    default:
      assert (! "unknown token in category `LC_CTYPE' should not happen");
    }

  for (cnt = 0; cnt < ctype->map_collection_nr; ++cnt)
    if (strcmp (name, ctype->mapnames[cnt]) == 0)
      break;

  if (cnt == ctype->map_collection_nr)
    assert (! "unknown token in category `LC_CTYPE' should not happen");

  ctype->last_map_idx = cnt;
  ctype->from_map_char = ILLEGAL_CHAR_VALUE;
}


void
ctype_map_from (struct linereader *lr, struct localedef_t *locale,
		struct token *code, struct charset_t *charset)
{
  struct locale_ctype_t *ctype = locale->categories[LC_CTYPE].ctype;
  unsigned int value;

  value = charset_find_value (&charset->char_table, code->val.str.start,
			      code->val.str.len);

  if ((wchar_t) value == ILLEGAL_CHAR_VALUE)
    /* In the LC_CTYPE category it is no error when a character is
       not found.  This has to be ignored silently.  */
    return;

  assert (ctype->last_map_idx < ctype->map_collection_nr);

  ctype->from_map_char = value;
}


void
ctype_map_to (struct linereader *lr, struct localedef_t *locale,
	      struct token *code, struct charset_t *charset)
{
  struct locale_ctype_t *ctype = locale->categories[LC_CTYPE].ctype;
  unsigned int value;

  value = charset_find_value (&charset->char_table, code->val.str.start,
			      code->val.str.len);

  if ((wchar_t) ctype->from_map_char == ILLEGAL_CHAR_VALUE
      || (wchar_t) value == ILLEGAL_CHAR_VALUE)
    {
      /* In the LC_CTYPE category it is no error when a character is
	 not found.  This has to be ignored silently.  */
      ctype->from_map_char = ILLEGAL_CHAR_VALUE;
      return;
    }

  *find_idx (ctype, &ctype->map_collection[ctype->last_map_idx],
	     &ctype->map_collection_max[ctype->last_map_idx],
	     &ctype->map_collection_act[ctype->last_map_idx],
	     ctype->from_map_char) = value;

  ctype->from_map_char = ILLEGAL_CHAR_VALUE;
}


void
ctype_map_end (struct linereader *lr, struct localedef_t *locale)
{
  struct locale_ctype_t *ctype = locale->categories[LC_CTYPE].ctype;

  ctype->last_map_idx = MAX_NR_CHARMAP;
  ctype->from_map_char = ILLEGAL_CHAR_VALUE;
}


/* Local functions.  */
static void
ctype_class_newP (struct linereader *lr, struct locale_ctype_t *ctype,
		  const char *name)
{
  size_t cnt;

  for (cnt = 0; cnt < ctype->nr_charclass; ++cnt)
    if (strcmp (ctype->classnames[cnt], name) == 0)
      break;

  if (cnt < ctype->nr_charclass)
    {
      lr_error (lr, _("character class `%s' already defined"), name);
      return;
    }

  if (ctype->nr_charclass == MAX_NR_CHARCLASS)
    /* Exit code 2 is prescribed in P1003.2b.  */
    error (2, 0, _("\
implementation limit: no more than %d character classes allowed"),
	   MAX_NR_CHARCLASS);

  ctype->classnames[ctype->nr_charclass++] = name;
}


static void
ctype_map_newP (struct linereader *lr, struct locale_ctype_t *ctype,
		const char *name, struct charset_t *charset)
{
  size_t max_chars = 0;
  size_t cnt;

  for (cnt = 0; cnt < ctype->map_collection_nr; ++cnt)
    {
      if (strcmp (ctype->mapnames[cnt], name) == 0)
	break;

      if (max_chars < ctype->map_collection_max[cnt])
	max_chars = ctype->map_collection_max[cnt];
    }

  if (cnt < ctype->map_collection_nr)
    {
      lr_error (lr, _("character map `%s' already defined"), name);
      return;
    }

  if (ctype->map_collection_nr == MAX_NR_CHARMAP)
    /* Exit code 2 is prescribed in P1003.2b.  */
    error (2, 0, _("\
implementation limit: no more than %d character maps allowed"),
	   MAX_NR_CHARMAP);

  ctype->mapnames[cnt] = name;

  if (max_chars == 0)
    ctype->map_collection_max[cnt] = charset->mb_cur_max == 1 ? 256 : 512;
  else
    ctype->map_collection_max[cnt] = max_chars;

  ctype->map_collection[cnt] = (u_int32_t *)
    xmalloc (sizeof (u_int32_t) * ctype->map_collection_max[cnt]);
  memset (ctype->map_collection[cnt], '\0',
	  sizeof (u_int32_t) * ctype->map_collection_max[cnt]);
  ctype->map_collection_act[cnt] = 256;

  ++ctype->map_collection_nr;
}


/* We have to be prepared that TABLE, MAX, and ACT can be NULL.  This
   is possible if we only want ot extend the name array.  */
static u_int32_t *
find_idx (struct locale_ctype_t *ctype, u_int32_t **table, size_t *max,
	  size_t *act, unsigned int idx)
{
  size_t cnt;

  if (idx < 256)
    return table == NULL ? NULL : &(*table)[idx];

  for (cnt = 256; cnt < ctype->charnames_act; ++cnt)
    if (ctype->charnames[cnt] == idx)
      break;

  /* We have to distinguish two cases: the names is found or not.  */
  if (cnt == ctype->charnames_act)
    {
      /* Extend the name array.  */
      if (ctype->charnames_act == ctype->charnames_max)
	{
	  ctype->charnames_max *= 2;
	  ctype->charnames = (unsigned int *)
	    xrealloc (ctype->charnames,
		      sizeof (unsigned int) * ctype->charnames_max);
	}
      ctype->charnames[ctype->charnames_act++] = idx;
    }

  if (table == NULL)
    /* We have done everything we are asked to do.  */
    return NULL;

  if (cnt >= *act)
    {
      if (cnt >= *max)
	{
	  size_t old_max = *max;
	  do
	    *max *= 2;
	  while (*max <= cnt);

	  *table =
	    (u_int32_t *) xrealloc (*table, *max * sizeof (unsigned long int));
	  memset (&(*table)[old_max], '\0',
		  (*max - old_max) * sizeof (u_int32_t));
	}

      (*table)[cnt] = 0;
      *act = cnt;
    }

  return &(*table)[cnt];
}


static void
set_class_defaults (struct locale_ctype_t *ctype, struct charset_t *charset)
{
  /* These function defines the default values for the classes and conversions
     according to POSIX.2 2.5.2.1.
     It may seem that the order of these if-blocks is arbitrary but it is NOT.
     Don't move them unless you know what you do!  */

  void set_default (int bit, int from, int to)
    {
      char tmp[2];
      int ch;
      /* Define string.  */
      strcpy (tmp, "?");

      for (ch = from; ch <= to; ++ch)
	{
	  unsigned int value;
	  tmp[0] = ch;

	  value = charset_find_value (&charset->char_table, tmp, 1);
	  if ((wchar_t) value == ILLEGAL_CHAR_VALUE)
	    {
	      if (!be_quiet)
		error (0, 0, _("\
character `%s' not defined while needed as default value"),
		       tmp);
	      continue;
	    }
	  else
	    ELEM (ctype, class_collection, , value) |= bit;
	}
    }

  /* Set default values if keyword was not present.  */
  if ((ctype->class_done & BIT (tok_upper)) == 0)
    /* "If this keyword [lower] is not specified, the lowercase letters
        `A' through `Z', ..., shall automatically belong to this class,
	with implementation defined character values."  [P1003.2, 2.5.2.1]  */
    set_default (BIT (tok_upper), 'A', 'Z');

  if ((ctype->class_done & BIT (tok_lower)) == 0)
    /* "If this keyword [lower] is not specified, the lowercase letters
        `a' through `z', ..., shall automatically belong to this class,
	with implementation defined character values."  [P1003.2, 2.5.2.1]  */
    set_default (BIT (tok_lower), 'a', 'z');

  if ((ctype->class_done & BIT (tok_alpha)) == 0)
    {
      /* Table 2-6 in P1003.2 says that characters in class `upper' or
	 class `lower' *must* be in class `alpha'.  */
      unsigned long int mask = BIT (tok_upper) | BIT (tok_lower);
      size_t cnt;

      for (cnt = 0; cnt < ctype->class_collection_act; ++cnt)
	if ((ctype->class_collection[cnt] & mask) != 0)
	  ctype->class_collection[cnt] |= BIT (tok_alpha);
    }

  if ((ctype->class_done & BIT (tok_digit)) == 0)
    /* "If this keyword [digit] is not specified, the digits `0' through
        `9', ..., shall automatically belong to this class, with
	implementation-defined character values."  [P1003.2, 2.5.2.1]  */
    set_default (BIT (tok_digit), '0', '9');

  /* "Only characters specified for the `alpha' and `digit' keyword
     shall be specified.  Characters specified for the keyword `alpha'
     and `digit' are automatically included in this class.  */
  {
    unsigned long int mask = BIT (tok_alpha) | BIT (tok_digit);
    size_t cnt;

    for (cnt = 0; cnt < ctype->class_collection_act; ++cnt)
      if ((ctype->class_collection[cnt] & mask) != 0)
	ctype->class_collection[cnt] |= BIT (tok_alnum);
  }

  if ((ctype->class_done & BIT (tok_space)) == 0)
    /* "If this keyword [space] is not specified, the characters <space>,
        <form-feed>, <newline>, <carriage-return>, <tab>, and
	<vertical-tab>, ..., shall automatically belong to this class,
	with implementation-defined character values."  [P1003.2, 2.5.2.1]  */
    {
      unsigned int value;

      value = charset_find_value (&charset->char_table, "space", 5);
      if ((wchar_t) value == ILLEGAL_CHAR_VALUE)
	{
	  if (!be_quiet)
	    error (0, 0, _("\
character `%s' not defined while needed as default value"),
		   "<space>");
	}
      else
	ELEM (ctype, class_collection, , value) |= BIT (tok_space);

      value = charset_find_value (&charset->char_table, "form-feed", 9);
      if ((wchar_t) value == ILLEGAL_CHAR_VALUE)
	{
	  if (!be_quiet)
	    error (0, 0, _("\
character `%s' not defined while needed as default value"),
		   "<form-feed>");
	}
      else
	ELEM (ctype, class_collection, , value) |= BIT (tok_space);

      value = charset_find_value (&charset->char_table, "newline", 7);
      if ((wchar_t) value == ILLEGAL_CHAR_VALUE)
	{
	  if (!be_quiet)
	    error (0, 0, _("\
character `%s' not defined while needed as default value"),
		   "<newline>");
	}
      else
	ELEM (ctype, class_collection, , value) |= BIT (tok_space);

      value = charset_find_value (&charset->char_table, "carriage-return", 15);
      if ((wchar_t) value == ILLEGAL_CHAR_VALUE)
	{
	  if (!be_quiet)
	    error (0, 0, _("\
character `%s' not defined while needed as default value"),
		   "<carriage-return>");
	}
      else
	ELEM (ctype, class_collection, , value) |= BIT (tok_space);

      value = charset_find_value (&charset->char_table, "tab", 3);
      if ((wchar_t) value == ILLEGAL_CHAR_VALUE)
	{
	  if (!be_quiet)
	    error (0, 0, _("\
character `%s' not defined while needed as default value"),
		   "<tab>");
	}
      else
	ELEM (ctype, class_collection, , value) |= BIT (tok_space);

      value = charset_find_value (&charset->char_table, "vertical-tab", 12);
      if ((wchar_t) value == ILLEGAL_CHAR_VALUE)
	{
	  if (!be_quiet)
	    error (0, 0, _("\
character `%s' not defined while needed as default value"),
		   "<vertical-tab>");
	}
      else
	ELEM (ctype, class_collection, , value) |= BIT (tok_space);
    }

  if ((ctype->class_done & BIT (tok_xdigit)) == 0)
    /* "If this keyword is not specified, the digits `0' to `9', the
        uppercase letters `A' through `F', and the lowercase letters `a'
	through `f', ..., shell automatically belong to this class, with
	implementation defined character values."  [P1003.2, 2.5.2.1]  */
    {
      set_default (BIT (tok_xdigit), '0', '9');
      set_default (BIT (tok_xdigit), 'A', 'F');
      set_default (BIT (tok_xdigit), 'a', 'f');
    }

  if ((ctype->class_done & BIT (tok_blank)) == 0)
    /* "If this keyword [blank] is unspecified, the characters <space> and
       <tab> shall belong to this character class."  [P1003.2, 2.5.2.1]  */
   {
      unsigned int value;

      value = charset_find_value (&charset->char_table, "space", 5);
      if ((wchar_t) value == ILLEGAL_CHAR_VALUE)
	{
	  if (!be_quiet)
	    error (0, 0, _("\
character `%s' not defined while needed as default value"),
		   "<space>");
	}
      else
	ELEM (ctype, class_collection, , value) |= BIT (tok_blank);

      value = charset_find_value (&charset->char_table, "tab", 3);
      if ((wchar_t) value == ILLEGAL_CHAR_VALUE)
	{
	  if (!be_quiet)
	    error (0, 0, _("\
character `%s' not defined while needed as default value"),
		   "<tab>");
	}
      else
	ELEM (ctype, class_collection, , value) |= BIT (tok_blank);
    }

  if ((ctype->class_done & BIT (tok_graph)) == 0)
    /* "If this keyword [graph] is not specified, characters specified for
        the keywords `upper', `lower', `alpha', `digit', `xdigit' and `punct',
	shall belong to this character class."  [P1003.2, 2.5.2.1]  */
    {
      unsigned long int mask = BIT (tok_upper) | BIT (tok_lower) |
	BIT (tok_alpha) | BIT (tok_digit) | BIT (tok_xdigit) | BIT (tok_punct);
      size_t cnt;

      for (cnt = 0; cnt < ctype->class_collection_act; ++cnt)
	if ((ctype->class_collection[cnt] & mask) != 0)
	  ctype->class_collection[cnt] |= BIT (tok_graph);
    }

  if ((ctype->class_done & BIT (tok_print)) == 0)
    /* "If this keyword [print] is not provided, characters specified for
        the keywords `upper', `lower', `alpha', `digit', `xdigit', `punct',
	and the <space> character shall belong to this character class."
	[P1003.2, 2.5.2.1]  */
    {
      unsigned long int mask = BIT (tok_upper) | BIT (tok_lower) |
	BIT (tok_alpha) | BIT (tok_digit) | BIT (tok_xdigit) | BIT (tok_punct);
      size_t cnt;
      wchar_t space;

      for (cnt = 0; cnt < ctype->class_collection_act; ++cnt)
	if ((ctype->class_collection[cnt] & mask) != 0)
	  ctype->class_collection[cnt] |= BIT (tok_print);

      space = charset_find_value (&charset->char_table, "space", 5);
      if (space == ILLEGAL_CHAR_VALUE)
	{
	  if (!be_quiet)
	    error (0, 0, _("\
character `%s' not defined while needed as default value"),
		   "<space>");
	}
      else
	ELEM (ctype, class_collection, , space) |= BIT (tok_print);
    }

  if (ctype->toupper_done == 0)
    /* "If this keyword [toupper] is not specified, the lowercase letters
        `a' through `z', and their corresponding uppercase letters `A' to
	`Z', ..., shall automatically be included, with implementation-
	defined character values."  [P1003.2, 2.5.2.1]  */
    {
      char tmp[4];
      int ch;

      strcpy (tmp, "<?>");

      for (ch = 'a'; ch <= 'z'; ++ch)
	{
	  unsigned int value_from, value_to;

	  tmp[1] = (char) ch;

	  value_from = charset_find_value (&charset->char_table, &tmp[1], 1);
	  if ((wchar_t) value_from == ILLEGAL_CHAR_VALUE)
	    {
	      if (!be_quiet)
		error (0, 0, _("\
character `%s' not defined while needed as default value"),
		       tmp);
	      continue;
	    }

	  /* This conversion is implementation defined.  */
	  tmp[1] = (char) (ch + ('A' - 'a'));
	  value_to = charset_find_value (&charset->char_table, &tmp[1], 1);
	  if ((wchar_t) value_to == ILLEGAL_CHAR_VALUE)
	    {
	      if (!be_quiet)
		error (0, 0, _("\
character `%s' not defined while needed as default value"),
		       tmp);
	      continue;
	    }

	  /* The index [0] is determined by the order of the
	     `ctype_map_newP' calls in `ctype_startup'.  */
	  ELEM (ctype, map_collection, [0], value_from) = value_to;
	}
    }

  if (ctype->tolower_done == 0)
    /* "If this keyword [tolower] is not specified, the mapping shall be
       the reverse mapping of the one specified to `toupper'."  [P1003.2]  */
    {
      size_t cnt;

      for (cnt = 0; cnt < ctype->map_collection_act[0]; ++cnt)
	if (ctype->map_collection[0][cnt] != 0)
	  ELEM (ctype, map_collection, [1],
		ctype->map_collection[0][cnt])
	    = ctype->charnames[cnt];
    }
}


static void
allocate_arrays (struct locale_ctype_t *ctype, struct charset_t *charset)
{
  size_t idx;

  /* First we have to decide how we organize the arrays.  It is easy
     for a one-byte character set.  But multi-byte character set
     cannot be stored flat because the chars might be sparsely used.
     So we determine an optimal hashing function for the used
     characters.

     We use a very trivial hashing function to store the sparse
     table.  CH % TABSIZE is used as an index.  To solve multiple hits
     we have N planes.  This guarantees a fixed search time for a
     character [N / 2].  In the following code we determine the minmum
     value for TABSIZE * N, where TABSIZE >= 256.  */
  size_t min_total = UINT_MAX;
  size_t act_size = 256;

  if (!be_quiet)
    fputs (_("\
Computing table size for character classes might take a while..."),
	   stderr);

  while (act_size < min_total)
    {
      size_t cnt[act_size];
      size_t act_planes = 1;

      memset (cnt, '\0', sizeof cnt);

      for (idx = 0; idx < 256; ++idx)
	cnt[idx] = 1;

      for (idx = 0; idx < ctype->charnames_act; ++idx)
	if (ctype->charnames[idx] >= 256)
	  {
	    size_t nr = ctype->charnames[idx] % act_size;

	    if (++cnt[nr] > act_planes)
	      {
		act_planes = cnt[nr];
		if (act_size * act_planes >= min_total)
		  break;
	      }
	  }

      if (act_size * act_planes < min_total)
	{
	  min_total = act_size * act_planes;
	  ctype->plane_size = act_size;
	  ctype->plane_cnt = act_planes;
	}

      ++act_size;
    }

  if (!be_quiet)
    fputs (_(" done\n"), stderr);


#if __BYTE_ORDER == __LITTLE_ENDIAN
# define NAMES_B1 ctype->names_el
# define NAMES_B2 ctype->names_eb
#else
# define NAMES_B1 ctype->names_eb
# define NAMES_B2 ctype->names_el
#endif

  ctype->names_eb = (u_int32_t *) xcalloc (ctype->plane_size
					   * ctype->plane_cnt,
					   sizeof (u_int32_t));
  ctype->names_el = (u_int32_t *) xcalloc (ctype->plane_size
					   * ctype->plane_cnt,
					   sizeof (u_int32_t));

  for (idx = 1; idx < 256; ++idx)
    NAMES_B1[idx] = idx;

  /* Trick: change the 0th entry's name to 1 to mark the cell occupied.  */
  NAMES_B1[0] = 1;

  for (idx = 256; idx < ctype->charnames_act; ++idx)
    {
      size_t nr = (ctype->charnames[idx] % ctype->plane_size);
      size_t depth = 0;

      while (NAMES_B1[nr + depth * ctype->plane_size])
	++depth;
      assert (depth < ctype->plane_cnt);

      NAMES_B1[nr + depth * ctype->plane_size] = ctype->charnames[idx];

      /* Now for faster access remember the index in the NAMES_B array.  */
      ctype->charnames[idx] = nr + depth * ctype->plane_size;
    }
  NAMES_B1[0] = 0;

  for (idx = 0; idx < ctype->plane_size * ctype->plane_cnt; ++idx)
    NAMES_B2[idx] = SWAPU32 (NAMES_B1[idx]);


  /* You wonder about this amount of memory?  This is only because some
     users do not manage to address the array with unsigned values or
     data types with range >= 256.  '\200' would result in the array
     index -128.  To help these poor people we duplicate the entries for
     128 up to 255 below the entry for \0.  */
  ctype->ctype_b = (char_class_t *) xcalloc (256 + 128,
					     sizeof (char_class_t));
  ctype->ctype32_b = (char_class32_t *) xcalloc (ctype->plane_size
						 * ctype->plane_cnt,
						 sizeof (char_class32_t));

  /* Fill in the character class information.  */
#if __BYTE_ORDER == __LITTLE_ENDIAN
# define TRANS(w) CHAR_CLASS_TRANS (w)
# define TRANS32(w) CHAR_CLASS32_TRANS (w)
#else
# define TRANS(w) (w)
# define TRANS32(w) (w)
#endif

  for (idx = 0; idx < ctype->class_collection_act; ++idx)
    if (ctype->charnames[idx] < 256)
      ctype->ctype_b[128 + ctype->charnames[idx]]
	= TRANS (ctype->class_collection[idx]);

  /* Mirror first 127 entries.  We must take care that entry -1 is not
     mirrored because EOF == -1.  */
  for (idx = 0; idx < 127; ++idx)
    ctype->ctype_b[idx] = ctype->ctype_b[256 + idx];

  /* The 32 bit array contains all characters.  */
  for (idx = 0; idx < ctype->class_collection_act; ++idx)
    ctype->ctype32_b[ctype->charnames[idx]]
      = TRANS32 (ctype->class_collection[idx]);

  /* Room for table of mappings.  */
  ctype->map_eb = (u_int32_t **) xmalloc (ctype->map_collection_nr
					  * sizeof (u_int32_t *));
  ctype->map_el = (u_int32_t **) xmalloc (ctype->map_collection_nr
					  * sizeof (u_int32_t *));

  /* Fill in all mappings.  */
  for (idx = 0; idx < ctype->map_collection_nr; ++idx)
    {
      unsigned int idx2;

      /* Allocate table.  */
      ctype->map_eb[idx] = (u_int32_t *) xmalloc ((ctype->plane_size
						   * ctype->plane_cnt + 128)
						  * sizeof (u_int32_t));
      ctype->map_el[idx] = (u_int32_t *) xmalloc ((ctype->plane_size
						   * ctype->plane_cnt + 128)
						  * sizeof (u_int32_t));

#if __BYTE_ORDER == __LITTLE_ENDIAN
# define MAP_B1 ctype->map_el
# define MAP_B2 ctype->map_eb
#else
# define MAP_B1 ctype->map_eb
# define MAP_B2 ctype->map_el
#endif

      /* Copy default value (identity mapping).  */
      memcpy (&MAP_B1[idx][128], NAMES_B1,
	      ctype->plane_size * ctype->plane_cnt * sizeof (u_int32_t));

      /* Copy values from collection.  */
      for (idx2 = 0; idx2 < ctype->map_collection_act[idx]; ++idx2)
	if (ctype->map_collection[idx][idx2] != 0)
	  MAP_B1[idx][128 + ctype->charnames[idx2]] =
	    ctype->map_collection[idx][idx2];

      /* Mirror first 127 entries.  We must take care not to map entry
	 -1 because EOF == -1.  */
      for (idx2 = 0; idx2 < 127; ++idx2)
	MAP_B1[idx][idx2] = MAP_B1[idx][256 + idx2];

      /* EOF must map to EOF.  */
      MAP_B1[idx][127] = EOF;

      /* And now the other byte order.  */
      for (idx2 = 0; idx2 < ctype->plane_size * ctype->plane_cnt + 128; ++idx2)
	MAP_B2[idx][idx2] = SWAPU32 (MAP_B1[idx][idx2]);
    }

  /* Extra array for class and map names.  */
  ctype->class_name_ptr = (u_int32_t *) xmalloc (ctype->nr_charclass
						 * sizeof (u_int32_t));
  ctype->map_name_ptr = (u_int32_t *) xmalloc (ctype->map_collection_nr
					       * sizeof (u_int32_t));

  /* Array for width information.  Because the expected width are very
     small we use only one single byte.  This save space and we need
     not provide the information twice with both endianesses.  */
  ctype->width = (unsigned char *) xmalloc (ctype->plane_size
					    * ctype->plane_cnt);
  /* Initialize with default width value.  */
  memset (ctype->width, charset->width_default,
	  ctype->plane_size * ctype->plane_cnt);
  if (charset->width_rules != NULL)
    {
      size_t cnt;

      for (cnt = 0; cnt < charset->nwidth_rules; ++cnt)
	if (charset->width_rules[cnt].width != charset->width_default)
	  for (idx = charset->width_rules[cnt].from;
	       idx <= charset->width_rules[cnt].to; ++idx)
	    {
	      size_t nr = idx % ctype->plane_size;
	      size_t depth = 0;

	      while (NAMES_B1[nr + depth * ctype->plane_size] != nr)
		++depth;
	      assert (depth < ctype->plane_cnt);

	      ctype->width[nr + depth * ctype->plane_size]
		= charset->width_rules[cnt].width;
	    }
    }

  /* Compute MB_CUR_MAX.  Please note the value mb_cur_max in the
     character set definition gives the number of bytes in the wide
     character representation.  We compute the number of bytes used
     for the UTF-8 encoded form.  */
  ctype->mb_cur_max = ((int []) { 2, 3, 5, 6 }) [charset->mb_cur_max - 1];

  /* We need the name of the currently used 8-bit character set to
     make correct conversion between this 8-bit representation and the
     ISO 10646 character set used internally for wide characters.  */
  ctype->codeset_name = charset->code_set_name;
}
