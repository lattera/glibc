/* Copyright (C) 1995 Free Software Foundation, Inc.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <alloca.h>
#include <fcntl.h>
#include <libintl.h>
#include <locale.h>
#include <localeinfo.h>
#include <langinfo.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/uio.h>

#include "localedef.h"
#include "token.h"

/* Arrays representing ctype tables.  They must be initialized for the
   right size to hold the full charmap.  */
static u16 *ctype_b;
static i32 *names_b, *toupper_b, *tolower_b;

/* For accessing the element of the (possibly sparse) array we use this
   macro.  */
#define ELEM(arr, idx)							     \
  (arr)[({ int h = idx % charmap_data.hash_size; 			     \
           int n = 0;							     \
           while (n < charmap_data.hash_layers                               \
	          && names_b[n * charmap_data.hash_size + h] != idx)         \
           ++n;								     \
           if (n >= charmap_data.hash_layers)				     \
             error (6, 0, gettext ("internal error in %s, line %u"),         \
		    __FUNCTION__, __LINE__);                                 \
           n * charmap_data.hash_size + h; })]

/* The bit used for representing a special class.  */
#define BITPOS(class) ((class) - TOK_UPPER)
#define BIT(class) (1 << BITPOS (class))

/* Remember which class or conversion is already done.  */
static unsigned short class_done = 0;
static unsigned short toupper_done = 0;
static unsigned short tolower_done = 0;

#define SYNTAX_ERROR                                                         \
    error (0, 0, gettext ("%s:%Zd: syntax error in locale definition file"), \
			  locfile_data.filename, locfile_data.line_no);

 
/* Prototypes for local functions.  */
static void allocate_arrays (void);
static void set_class_defaults (void);
static int valid_char (int ch);


/* Read CTYPE category.  The initial token is given as a parameter.  */
void
ctype_input (int token)
{
  char *ptr;
  int len;

  /* If necessary allocate arrays.  */
  allocate_arrays ();

  while (token != TOK_END)
    {
      switch (token)
	{
	case TOK_UPPER:  case TOK_LOWER: case TOK_ALPHA: case TOK_DIGIT:
	case TOK_XDIGIT: case TOK_SPACE: case TOK_PRINT: case TOK_GRAPH:
	case TOK_BLANK:  case TOK_CNTRL: case TOK_PUNCT:
	  {
	    /* TAKE CARE: the order of the tokens in "token.h" determines
	       the bit used to indicate the membership in the class.  This
	       also has to correspond to the values used in <ctype.h>.  */
	    int bit = BIT (token);
	    int was_ell = 0;
	    int last = -1;

	    if ((class_done & bit) != 0)
	      {
		char tmp[len + 1];
		memcpy (tmp, ptr, len);
		tmp[len] = '\0';

		error (0, 0, gettext ("%s:%Zd: duplicate definiton of item "
				      "`%s' in category `LC_CTYPE'"),
		       locfile_data.filename, locfile_data.line_no, tmp);
	      }
	    class_done |= bit;

	    do
	      {
		token = xlocfile_lex (&ptr, &len);

		if (token == TOK_ENDOFLINE)
		  {
		    SYNTAX_ERROR;
		    break;
		  }

		if (token == TOK_ELLIPSIS)
		  {
		    if (was_ell != 0 || last < 0)
		      {
			error (0, 0, gettext ("%s:%Zd: illegal use of `...'"),
			       locfile_data.filename, locfile_data.line_no);
			break;
		      }
		    was_ell = 1;
		    continue;
		  }

		if (token != TOK_CHAR)
		  {
		    if (token != TOK_ILL_CHAR)
		      SYNTAX_ERROR;
		    was_ell = 0;
		    last = -1;
		    continue;
		  }

		if (len < 0 || !valid_char (len))
		  {
		    was_ell = 0;
		    last = -1;
		    continue;
		  }

		/* We have found a valid character.  Include it to
		   the class' bit set.  */
		if (was_ell == 0)
		  {
		    ELEM (ctype_b, len) |= bit;
		    last = len;
		  }
		else
		  {
		    int i;

		    if (last > len)
		      {
			error (0, 0, gettext ("%s:%Zd: lower bound of "
					      "ellipsis not smaller"),
			       locfile_data.filename, locfile_data.line_no);
			was_ell = 0;
			last = -1;
			continue;
		      }

		    for (i = last + 1; i <= len; ++i)
		      ELEM (ctype_b, i) |= bit;

		    last = -1;
		  }
		was_ell = 0;
	      }
	    while ((token = locfile_lex (&ptr, &len)) == TOK_CHAR
		   && len == ';');

	    /* Rest of the line should be empty.  */
	    ignore_to_eol (token, 0);
	  }
	  break;
	case TOK_TOUPPER: case TOK_TOLOWER:
	  {
	    int from;
	    int to = -1;
	    int is_upper = token == TOK_TOUPPER;

	    if (((is_upper ? toupper_done : tolower_done) & BIT (token)) != 0)
	      error (0, 0, gettext ("%s:%Zd: duplicate definition of item "
				    "`%s' in category `LC_CTYPE'"),
		     locfile_data.filename, locfile_data.line_no,
		     is_upper ? "toupper" : "tolower");
	    (is_upper ? toupper_done : tolower_done) |= BIT (token); 

	    do
	      {
		int ignore;

		token = xlocfile_lex (&ptr, &len);
		if (token != TOK_CHAR || len != '(')
		  {
		    SYNTAX_ERROR;
		    break;
		  }

		token = xlocfile_lex (&ptr, &len);
		if (token != TOK_CHAR && token != TOK_ILL_CHAR)
		  {
		    SYNTAX_ERROR;
		    break;
		  }
		from = len;
		ignore = token == TOK_ILL_CHAR;

		token = xlocfile_lex (&ptr, &len);
		if (token != TOK_CHAR || len != ',')
		  {
		    SYNTAX_ERROR;
		    break;
		  }

		token = xlocfile_lex (&ptr, &len);
		if (token != TOK_CHAR && token != TOK_ILL_CHAR)
		  {
		    SYNTAX_ERROR;
		    break;
		  }
		to = len;
		ignore |= token == TOK_ILL_CHAR;
	      
		token = xlocfile_lex (&ptr, &len);
		if (token != TOK_CHAR || len != ')')
		  {
		    SYNTAX_ERROR;
		    break;
		  }

		if (!ignore && valid_char (from) && valid_char (to))
		  /* Have a valid pair.  */
		  ELEM (is_upper ? toupper_b : tolower_b, from) = to;
	      }
	    while ((token = locfile_lex (&ptr, &len)) == TOK_CHAR
		   && len == ';');

	    /* Rest of the line should be empty.  */
	    ignore_to_eol (token, 1);
	  }
	  break;
	default:
	  SYNTAX_ERROR;
	  ignore_to_eol (0, 0);
	  break;
	}

      /* Read next token.  */
      token = xlocfile_lex (&ptr, &len);
    }

  token = xlocfile_lex (&ptr, &len);

  if (token != _NL_NUM_LC_CTYPE)
    {
      error (0, 0, gettext ("%s:%Zd: category `%s' does not end with "
			    "`END %s'"), locfile_data.filename,
	     locfile_data.line_no, "LC_CTYPE", "LC_CTYPE");
      ignore_to_eol (0, 0);
    }
  else
    ignore_to_eol (0, posix_conformance);
}


void
ctype_check(void)
{
  /* Here are a lot of things to check.  See POSIX.2, table 2-6.  */
  #define NCLASS 11
  static const struct
    {
      const char *name;
      const char allow[NCLASS];
    }
  valid_table[NCLASS] =
    {
      /* The order is important.  See token.h for more information.
         M = Always, D = Default, - = Permitted, X = Mutually exclusive  */
      [BITPOS (TOK_UPPER)]  = { "upper",  "--MX-XDDXXX" },
      [BITPOS (TOK_LOWER)]  = { "lower",  "--MX-XDDXXX" },
      [BITPOS (TOK_ALPHA)]  = { "alpha",  "---X-XDDXXX" },
      [BITPOS (TOK_DIGIT)]  = { "digit",  "XXX--XDDXXX" },
      [BITPOS (TOK_XDIGIT)] = { "xdigit", "-----XDDXXX" },
      [BITPOS (TOK_SPACE)]  = { "space",  "XXXXX------" },
      [BITPOS (TOK_PRINT)]  = { "print",  "---------X-" },
      [BITPOS (TOK_GRAPH)]  = { "graph",  "---------X-" },
      [BITPOS (TOK_BLANK)]  = { "blank",  "XXXXXM-----" },
      [BITPOS (TOK_CNTRL)]  = { "cntrl",  "XXXXX-XX--X" },
      [BITPOS (TOK_PUNCT)]  = { "punct",  "XXXXX-DD-X-" }
    };
  int ch, cls1, cls2, eq, space_char;
  u16 tmp;

  /* Set default value for classes not specified.  */
  set_class_defaults ();

  /* Check according to table.  */
  for (ch = 0; ch < charmap_data.hash_size * charmap_data.hash_layers; ++ch)
    {
      if (ch != 0 && names_b[ch] == 0)
	continue;
      tmp = ELEM (ctype_b, names_b[ch]);
      for (cls1 = 0; cls1 < NCLASS; ++cls1)
        if ((tmp & (1 << cls1)) != 0)
          for (cls2 = 0; cls2 < NCLASS; ++cls2)
            if (cls2 != cls1 && valid_table[cls1].allow[cls2] != '-')
              {
                eq = (tmp & (1 << cls2)) != 0;
                switch (valid_table[cls1].allow[cls2])
                  {
                  case 'M':
                    if (!eq)
                      error (0, 0, gettext ("character '\\%o' in class `%s' "
					    "must be in class `%s'"), ch,
			     valid_table[cls1].name, valid_table[cls2].name);
                    break;
                  case 'X':
                    if (eq)
                      error (0, 0, gettext ("character '\\%o' inc class `%s' "
					    "must not be in class `%s'"), ch,
			     valid_table[cls1].name, valid_table[cls2].name);
                    break;
                  case 'D':
                    ELEM (ctype_b, names_b[ch]) |= 1 << cls2;
                    break;
                  default:
                    error (5, 0, gettext ("internal error in %s, line %u"),
			   __FUNCTION__, __LINE__);
                  }
              }
    }

  /* ... and now test <SP>  as a special case.  */
  if (find_entry (&charmap_data.table, "SP", 2, (void **) &space_char) == 0)
    error (0, 0, gettext ("character <SP> not defined in character map"));
  else if ((tmp = BITPOS (TOK_SPACE),
            (ELEM (ctype_b, space_char) & BIT (TOK_SPACE)) == 0)
           || (tmp = BITPOS (TOK_BLANK),
               (ELEM (ctype_b, space_char) & BIT (TOK_BLANK)) == 0))
    error (0, 0, gettext ("<SP> character not in class `%s'"),
	   valid_table[tmp].name);
  else if ((tmp = BITPOS (TOK_PUNCT),
            (ELEM (ctype_b, space_char) & BIT (TOK_PUNCT)) != 0)
           || (tmp = BITPOS (TOK_GRAPH),
               (ELEM (ctype_b, space_char) & BIT (TOK_GRAPH)) != 0))
    error (0, 0, gettext ("<SP> character must not be in class `%s'"),
	   valid_table[tmp].name);
  else
    ELEM (ctype_b, space_char) |= BIT (TOK_PRINT);
}


/* These macros can change little to big endian and vice versa.  */
#define SWAP16(v)							     \
      ((u16) (((((unsigned short) (v)) & 0x00ff) << 8)			     \
	      | ((((unsigned short) (v)) & 0xff00) >> 8)))
#define SWAP32(v)							     \
	((u32) (((((u32) (v)) & 0x000000ff) << 24)			     \
		| ((((u32) (v)) & 0x0000ff00) << 8)			     \
		| ((((u32) (v)) & 0x00ff0000) >> 8)			     \
		| ((((u32) (v)) & 0xff000000) >> 24)))


int
ctype_output (void)
{
  char *path, *t;
  int ch;
  /* File descriptor for output file.  */
  int fd;
  /* Magic number.  */
  i32 magic = LIMAGIC (LC_CTYPE);
  /* Number of table.  */
  int tables = 6;
  /* Number ints in leading information table.  */
#if 0
  i32 n = 2 + 2 * tables;
#else
  i32 n = 5;
#endif
  /* Values describing the character set.  */
  char mb_cur_min = (char) charmap_data.mb_cur_min;
  char mb_cur_max = (char) charmap_data.mb_cur_max;
  /* Optimal size of hashing table.  */
  i32 hash_size = charmap_data.hash_size;
  i32 hash_layers = charmap_data.hash_layers;
  /* Number of elements in the tables.  */
  int size = hash_size * charmap_data.hash_layers;
  /* Positions of the tables.  */
  i32 pos[14] =
    {
      /* No, no.  We don't play towers of Hanoi.  This is a more or less
	 readable table of the offsets of the different strings in the
	 produced file.  It is seperated in three columns which represent
	 the number of values with 1, 2, and 4 bytes.  */

#if 0
                                   4 *  (2 + n),
      1 +                          4 *  (2 + n),
      2 +                          4 *  (2 + n),
      2 +                          4 *  (3 + n),
      2 +                          4 *  (4 + n),
      2 + 2 *      (128 + size)  + 4 *  (4 + n),
      2 + 2 *      (128 + size)  + 4 * ((4 + n) +     (size + 128)),
      2 + 2 *      (128 + size)  + 4 * ((4 + n) + 2 * (size + 128)),
      2 + 2 *      (128 + size)  + 4 * ((4 + n) + 2 * (size + 128) + 1 * size),
      2 + 2 *      (128 + size)  + 4 * ((5 + n) + 2 * (size + 128) + 1 * size),
      2 + 2 *      (128 + size)  + 4 * ((6 + n) + 2 * (size + 128) + 1 * size),
      2 + 2 * (2 * (128 + size)) + 4 * ((6 + n) + 2 * (size + 128) + 1 * size),
      2 + 2 * (2 * (128 + size)) + 4 * ((6 + n) + 3 * (size + 128) + 1 * size),
      2 + 2 * (2 * (128 + size)) + 4 * ((6 + n) + 4 * (size + 128) + 1 * size),
#else
                                   4 *  (2 + n),
          2 *      (128 + size)  + 4 *  (2 + n),
          2 *      (128 + size)  + 4 * ((2 + n) +     (size + 128)),
          2 *      (128 + size)  + 4 * ((2 + n) + 2 * (size + 128)),
	  2 *      (128 + size)  + 4 * ((2 + n) + 3 * (size + 128)),
#endif
    };
  /* Parameter to writev.  */
  struct iovec iov[11] = 
    { 
      { &magic, sizeof (i32) },
      { &n, sizeof (i32) },
#if 0
      { pos, sizeof (pos) },
      { &mb_cur_min, 1 },
      { &mb_cur_max, 1 },
      { &hash_size, sizeof (i32) },
      { &hash_layers, sizeof (i32) },
#else
      { pos, 5 * 4 },
#endif
      { ctype_b   - 128, (size + 128) * sizeof (u16) },
      { toupper_b - 128, (size + 128) * sizeof (i32) },
      { tolower_b - 128, (size + 128) * sizeof (i32) },
      { names_b, size * sizeof (i32) }
    };
  int result = 0;
  
  /* Now we can bring the representations into the right form.  */
  for (ch = -128; ch < -1; ++ch)
    {
      ctype_b[ch] = ctype_b[256 + ch];
      toupper_b[ch] = toupper_b[256 + ch];
      tolower_b[ch] = tolower_b[256 + ch];
    }
  /* Set value for EOF.  */
  ctype_b[-1] = 0;
  toupper_b[-1] = -1;
  tolower_b[-1] = -1;

  for (ch = -128; ch < size; ++ch)
    ctype_b[ch] = htons (ctype_b[ch]);

  /* Construct the output filename from the argument given to
     localedef on the command line.  */
  path = (char *) alloca (strlen (output_path) +
			  strlen (category[LC_CTYPE].name) + 1);
  t = stpcpy (path, output_path);
  strcpy (t, category[LC_CTYPE].name);

  fd = creat (path, 0666);
  if (fd == -1)
    {
      error (0, 0, gettext ("cannot open output file `%s': %m"), path);
      result = 1;
    }
  else
    {
      int idx;

#if 0
      if (writev (fd, iov, 10) == -1)
#else
      if (writev (fd, iov, 6) == -1)
#endif
	{
	  error (0, 0, gettext ("cannot write output file `%s': %m"), path);
	  result = 1;
	  goto close_and_return;
	}

      /* Now we have to write the three tables with different endianess.  */
      hash_size = SWAP32 (hash_size);
      for (idx = -128; idx < size; ++idx)
	{
	  ctype_b[idx] = SWAP16 (ctype_b[idx]);
	  toupper_b[idx] = SWAP32 (toupper_b[idx]);
	  tolower_b[idx] = SWAP32 (tolower_b[idx]);
	  if (idx >= 0)
	    names_b[idx] = SWAP32 (names_b[idx]);
	}

#if 0
      if (writev (fd, iov + 5, 6) == -1)
#else
      if (writev (fd, iov + 3, 2) == -1)
#endif
	{
	  error (0, 0, gettext ("cannot write output file `%s': %m"), path);
	  result = 1;
	}

      close_and_return:
      close (fd);
    }

  return result;
}


/* If necessary allocate the memory for the arrays according to the
   current character map.  */
static void
allocate_arrays (void)
{
  /* Init ctype data structures.  */
  if (ctype_b == NULL)
    /* All data structures are not initialized yet.  */
    {
      /* You wonder about this amount of memory?  This is only because
	 some users do not manage to address the array with unsigned
	 values or data types with range >= 256.  '\200' would result
	 in the array index -128.  To help these poor people we
	 duplicate the entries for 128 upto 255 below the entry for \0.  */
      int ch, h, n;
      char *ptr;
      int size = charmap_data.hash_size * charmap_data.hash_layers;

      ctype_b = (u16 *) xcalloc (size - (-128), sizeof (u16));
      ctype_b += 128;


      names_b = (i32 *) xcalloc (size, sizeof (i32));

      toupper_b = (i32 *) xcalloc ((size - (-128)), sizeof  (i32));
      toupper_b += 128;

      tolower_b = (i32 *) xcalloc ((size - (-128)), sizeof (i32));
      tolower_b += 128;

      ptr = NULL;
      /* Mark the place of the NUL character as occupied.  */
      names_b[0] = 1;

      while (iterate_table (&charmap_data.table, (void **) &ptr,
			    (void **) &ch))
	{
	  /* We already handled the NUL character.  */
	  if (ch == 0)
	    continue;

	  h = ch % charmap_data.hash_size;
	  n = 0;
	  while (names_b[h + n * charmap_data.hash_size] != 0)
	    ++n;

	  names_b[h + n * charmap_data.hash_size] = ch;
	  toupper_b[h + n * charmap_data.hash_size] = ch;
	  tolower_b[h + n * charmap_data.hash_size] = ch;
	}
      /* Correct the value for NUL character.  */
      names_b[0] = 0;
    }
}

static void
set_class_defaults (void)
{
  /* These function defines the default values for the classes and conversions
     according to POSIX.2 2.5.2.1.
     It may seem that the order of these if-blocks is arbitrary but it is NOT.
     Don't move them unless you know what you do!  */

  void set_default (int bit, int from, int to)
    {
      char tmp[4];
      int ch;
      /* Define string.  */
      strcpy (tmp, "<?>");

      for (ch = from; ch <= to; ++ch)
	{
	  int code;
	  tmp[1] = ch;

	  code = find_char (tmp + 1, 1);
	  if (code == -1)
	    error (5, 0, gettext ("character `%s' not defined while needed "
				  "as default value"), tmp);
	  ELEM (ctype_b, code) |= bit;
	}
    }

  /* If necessary allocate arrays.  */
  allocate_arrays ();

  /* Set default values if keyword was not present.  */
  if ((class_done & BIT (TOK_UPPER)) == 0)
    /* "If this keyword [lower] is not specified, the lowercase letters
        `A' through `Z', ..., shall automatically belong to this class,
	with implementation defined character values."  */
    set_default (BIT (TOK_UPPER), 'A', 'Z');

  if ((class_done & BIT (TOK_LOWER)) == 0)
    /* "If this keyword [lower] is not specified, the lowercase letters
        `a' through `z', ..., shall automatically belong to this class,
	with implementation defined character values."  */
    set_default (BIT (TOK_LOWER), 'a', 'z');

  if ((class_done & BIT (TOK_DIGIT)) == 0)
    /* "If this keyword [digit] is not specified, the digits `0' through
        `9', ..., shall automatically belong to this class, with
	implementation-defined character values."  */        
    set_default (BIT (TOK_DIGIT), '0', '9');

  if ((class_done & BIT (TOK_SPACE)) == 0)
    /* "If this keyword [space] is not specified, the characters <space>,
        <form-feed>, <newline>, <carriage-return>, <tab>, and
	<vertical-tab>, ..., shall automatically belong to this class,
	with implementtation-defined character values."  */
    {
      int code;

      code = find_char ("space", 5);
      if (code == -1)
	error (5, 0, gettext ("character `%s' not defined while needed as "
			      "default value"), "<space>");
      ELEM (ctype_b, code) |= BIT (TOK_SPACE);

      code = find_char ("form-feed", 9);
      if (code == -1)
	error (5, 0, gettext ("character `%s' not defined while needed as "
			      "default value"), "<form-feed>");
      ELEM (ctype_b, code) |= BIT (TOK_SPACE);

      code = find_char ("newline", 7);
      if (code == -1)
	error (5, 0, gettext ("character `%s' not defined while needed as "
			      "default value"), "<newline>");
      ELEM (ctype_b, code) |= BIT (TOK_SPACE);

      code = find_char ("carriage-return", 15);
      if (code == -1)
	error (5, 0, gettext ("character `%s' not defined while needed as "
			      "default value"), "<carriage-return>");
      ELEM (ctype_b, code) |= BIT (TOK_SPACE);

      code = find_char ("tab", 3);
      if (code == -1)
	error (5, 0, gettext ("character `%s' not defined while needed as "
			      "default value"), "<tab>");
      ELEM (ctype_b, code) |= BIT (TOK_SPACE);

      code = find_char ("vertical-tab", 11);
      if (code == -1)
	error (5, 0, gettext ("character `%s' not defined while needed as "
			      "default value"), "<vertical-tab>");
      ELEM (ctype_b, code) |= BIT (TOK_SPACE);
    }
  
  if ((class_done & BIT (TOK_XDIGIT)) == 0)
    /* "If this keyword is not specified, the digits `0' to `9', the
        uppercase letters `A' through `F', and the lowercase letters `a'
	through `f', ..., shell automatically belong to this class, with
	implementation defined character values."  */
    {
      if ((class_done & BIT (TOK_XDIGIT)) == 0)
	set_default (BIT (TOK_XDIGIT), '0', '9');

      if ((class_done & BIT (TOK_XDIGIT)) == 0)
	set_default (BIT (TOK_XDIGIT), 'A', 'F');

      if ((class_done & BIT (TOK_XDIGIT)) == 0)
	set_default (BIT (TOK_XDIGIT), 'a', 'f');
    }

  if ((class_done & BIT (TOK_BLANK)) == 0)
    /* "If this keyword [blank] is unspecified, the characters <space> and
       <tab> shall belong to this character class."  */
   {
      int code;

      code = find_char ("space", 5);
      if (code == -1)
	error (5, 0, gettext ("character `%s' not defined while needed as "
			      "default value"), "<space>");
      ELEM (ctype_b, code) |= BIT (TOK_BLANK);

      code = find_char ("tab", 3);
      if (code == -1)
	error (5, 0, gettext ("character `%s' not defined while needed as "
			      "default value"), "<tab>");
      ELEM (ctype_b, code) |= BIT (TOK_BLANK);
    }

  if ((class_done & BIT (TOK_GRAPH)) == 0)
    /* "If this keyword [graph] is not specified, characters specified for
        the keywords `upper', `lower', `alpha', `digit', `xdigit' and `punct',
	shall belong to this character class."  */
    {
      int ch;
      unsigned short int mask = BIT (TOK_UPPER) | BIT (TOK_LOWER) |
	BIT (TOK_ALPHA) | BIT (TOK_DIGIT) | BIT (TOK_XDIGIT) | BIT (TOK_PUNCT);

      for (ch = 0; ch < charmap_data.hash_size * charmap_data.hash_layers;
		   ++ch)
	{
	  if (ch != 0 && names_b[ch] == 0)
	    continue;
	  if ((ELEM (ctype_b, names_b[ch]) & mask) != 0)
	    ELEM (ctype_b, names_b[ch]) |= BIT (TOK_GRAPH);
	}
    }

  if ((class_done & BIT (TOK_PRINT)) == 0)
    /* "If this keyword [print] is not provided, characters specified for
        the keywords `upper', `lower', `alpha', `digit', `xdigit', `punct',
	and the <space> character shall belong to this character class."  */
    {
      int ch;
      int space = find_char ("space", 5);
      unsigned short int mask = BIT (TOK_UPPER) | BIT (TOK_LOWER) |
	BIT (TOK_ALPHA) | BIT (TOK_DIGIT) | BIT (TOK_XDIGIT) | BIT (TOK_PUNCT);

      if (space == -1)
	error (5, 0, gettext ("character `%s' not defined while needed as "
			      "default value"), "<space>");

      for (ch = 0; ch < charmap_data.hash_size * charmap_data.hash_layers;
		   ++ch)
	{
	  if (ch != 0 && names_b[ch] == 0)
	    continue;
	  if ((ELEM (ctype_b, names_b[ch]) & mask) != 0)
	    ELEM (ctype_b, names_b[ch]) |= BIT (TOK_PRINT);
	}
      ELEM (ctype_b, space) |= BIT (TOK_PRINT);
    }

  if (toupper_done == 0)
    /* "If this keyword [toupper] is not spcified, the lowercase letters
        `a' through `z', and their corresponding uppercase letters `A' to
	`Z', ..., shall automatically be included, with implementation-
	defined character values."  */
    {
      char tmp[4];
      int ch;

      strcpy (tmp, "<?>");

      for (ch = 'a'; ch <= 'z'; ++ch)
	{
	  int code_to, code_from;

	  tmp[1] = ch;
	  code_from = find_char (tmp + 1, 1);
	  if (code_from == -1)
	    error (5, 0, gettext ("character `%s' not defined while needed "
				  "as default value"), tmp);

	  /* This conversion is implementation defined.  */
	  tmp[1] = ch + ('A' - 'a');
	  code_to = find_char (tmp + 1, 1);
	  if (code_to == -1)
	    error (5, 0, gettext ("character `%s' not defined while needed "
				  "as default value"), tmp);

	  ELEM (toupper_b, code_from) = code_to;
	}
    }

  if (tolower_done == 0)
    /* "If this keyword [tolower] is not specified, the mapping shall be
        the reverse mapping of the one specified to `toupper'."  */
    {
      int ch;

      for (ch = 0; ch < charmap_data.hash_size * charmap_data.hash_layers;
		   ++ch)
	{
	  if (ch != 0 && names_b[ch] == 0)
	    continue;

	  if (toupper_b[ch] != names_b[ch])
	    ELEM (tolower_b, toupper_b[ch]) = names_b[ch];
	}
    }
}


/* Test whether the given character is valid for the current charmap.  */
static int
valid_char (int ch)
{
  /* FIXME: this assumes 32-bit integers.  */
  int ok = ch >= 0
    && (charmap_data.mb_cur_max < 4
	? ch < 1 << (8 * charmap_data.mb_cur_max) : 1);

  return ok;
}


/*
 * Local Variables:
 *  mode:c
 *  c-basic-offset:2
 * End:
 */
