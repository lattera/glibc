/* Copyright (C) 1991-1993, 1996-2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* Match STRING against the filename pattern PATTERN, returning zero if
   it matches, nonzero if not.  */
static int FCT (const CHAR *pattern, const CHAR *string,
		int no_leading_period, int flags) internal_function;

static int
internal_function
FCT (pattern, string, no_leading_period, flags)
     const CHAR *pattern;
     const CHAR *string;
     int no_leading_period;
     int flags;
{
  register const CHAR *p = pattern, *n = string;
  register UCHAR c;
#ifdef _LIBC
# if WIDE_CHAR_VERSION
  const char *collseq = (const char *)
    _NL_CURRENT(LC_COLLATE, _NL_COLLATE_COLLSEQWC);
# else
  const UCHAR *collseq = (const UCHAR *)
    _NL_CURRENT(LC_COLLATE, _NL_COLLATE_COLLSEQMB);
# endif
#endif

  while ((c = *p++) != L('\0'))
    {
      c = FOLD (c);

      switch (c)
	{
	case L('?'):
	  if (*n == L('\0'))
	    return FNM_NOMATCH;
	  else if (*n == L('/') && (flags & FNM_FILE_NAME))
	    return FNM_NOMATCH;
	  else if (*n == L('.') && no_leading_period
		   && (n == string
		       || (n[-1] == L('/') && (flags & FNM_FILE_NAME))))
	    return FNM_NOMATCH;
	  break;

	case L('\\'):
	  if (!(flags & FNM_NOESCAPE))
	    {
	      c = *p++;
	      if (c == L('\0'))
		/* Trailing \ loses.  */
		return FNM_NOMATCH;
	      c = FOLD (c);
	    }
	  if (FOLD ((UCHAR) *n) != c)
	    return FNM_NOMATCH;
	  break;

	case L('*'):
	  if (*n == L('.') && no_leading_period
	      && (n == string
		  || (n[-1] == L('/') && (flags & FNM_FILE_NAME))))
	    return FNM_NOMATCH;

	  for (c = *p++; c == L('?') || c == L('*'); c = *p++)
	    {
	      if (*n == L('/') && (flags & FNM_FILE_NAME))
		/* A slash does not match a wildcard under FNM_FILE_NAME.  */
		return FNM_NOMATCH;
	      else if (c == L('?'))
		{
		  /* A ? needs to match one character.  */
		  if (*n == L('\0'))
		    /* There isn't another character; no match.  */
		    return FNM_NOMATCH;
		  else
		    /* One character of the string is consumed in matching
		       this ? wildcard, so *??? won't match if there are
		       less than three characters.  */
		    ++n;
		}
	    }

	  if (c == L('\0'))
	    /* The wildcard(s) is/are the last element of the pattern.
	       If the name is a file name and contains another slash
	       this means it cannot match, unless the FNM_LEADING_DIR
	       flag is set.  */
	    {
	      int result = (flags & FNM_FILE_NAME) == 0 ? 0 : FNM_NOMATCH;

	      if (flags & FNM_FILE_NAME)
		{
		  if (flags & FNM_LEADING_DIR)
		    result = 0;
		  else
		    {
		      if (STRCHR (n, L('/')) == NULL)
			result = 0;
		    }
		}

	      return result;
	    }
	  else
	    {
	      const CHAR *endp;

	      endp = STRCHRNUL (n, (flags & FNM_FILE_NAME) ? L('/') : L('\0'));

	      if (c == L('['))
		{
		  int flags2 = ((flags & FNM_FILE_NAME)
				? flags : (flags & ~FNM_PERIOD));

		  for (--p; n < endp; ++n)
		    if (FCT (p, n, (no_leading_period
				    && (n == string
					|| (n[-1] == L('/')
					    && (flags & FNM_FILE_NAME)))),
			     flags2) == 0)
		      return 0;
		}
	      else if (c == L('/') && (flags & FNM_FILE_NAME))
		{
		  while (*n != L('\0') && *n != L('/'))
		    ++n;
		  if (*n == L('/')
		      && (FCT (p, n + 1, flags & FNM_PERIOD, flags) == 0))
		    return 0;
		}
	      else
		{
		  int flags2 = ((flags & FNM_FILE_NAME)
				? flags : (flags & ~FNM_PERIOD));

		  if (c == L('\\') && !(flags & FNM_NOESCAPE))
		    c = *p;
		  c = FOLD (c);
		  for (--p; n < endp; ++n)
		    if (FOLD ((UCHAR) *n) == c
			&& (FCT (p, n, (no_leading_period
					&& (n == string
					    || (n[-1] == L('/')
						&& (flags & FNM_FILE_NAME)))),
				 flags2) == 0))
		      return 0;
		}
	    }

	  /* If we come here no match is possible with the wildcard.  */
	  return FNM_NOMATCH;

	case L('['):
	  {
	    static int posixly_correct;
	    /* Nonzero if the sense of the character class is inverted.  */
	    register int not;
	    CHAR cold;

	    if (posixly_correct == 0)
	      posixly_correct = getenv ("POSIXLY_CORRECT") != NULL ? 1 : -1;

	    if (*n == L('\0'))
	      return FNM_NOMATCH;

	    if (*n == L('.') && no_leading_period
		&& (n == string
		    || (n[-1] == L('/') && (flags & FNM_FILE_NAME))))
	      return FNM_NOMATCH;

	    if (*n == L('/') && (flags & FNM_FILE_NAME))
	      /* `/' cannot be matched.  */
	      return FNM_NOMATCH;

	    not = (*p == L('!') || (posixly_correct < 0 && *p == L('^')));
	    if (not)
	      ++p;

	    c = *p++;
	    for (;;)
	      {
		UCHAR fn = FOLD ((UCHAR) *n);

		if (!(flags & FNM_NOESCAPE) && c == L('\\'))
		  {
		    if (*p == L('\0'))
		      return FNM_NOMATCH;
		    c = FOLD ((UCHAR) *p);
		    ++p;

		    if (c == fn)
		      goto matched;
		  }
		else if (c == L('[') && *p == L(':'))
		  {
		    /* Leave room for the null.  */
		    CHAR str[CHAR_CLASS_MAX_LENGTH + 1];
		    size_t c1 = 0;
#if defined _LIBC || (defined HAVE_WCTYPE_H && defined HAVE_WCHAR_H)
		    wctype_t wt;
#endif
		    const CHAR *startp = p;

		    for (;;)
		      {
			if (c1 == CHAR_CLASS_MAX_LENGTH)
			  /* The name is too long and therefore the pattern
			     is ill-formed.  */
			  return FNM_NOMATCH;

			c = *++p;
			if (c == L(':') && p[1] == L(']'))
			  {
			    p += 2;
			    break;
			  }
			if (c < L('a') || c >= L('z'))
			  {
			    /* This cannot possibly be a character class name.
			       Match it as a normal range.  */
			    p = startp;
			    c = L('[');
			    goto normal_bracket;
			  }
			str[c1++] = c;
		      }
		    str[c1] = L('\0');

#if defined _LIBC || (defined HAVE_WCTYPE_H && defined HAVE_WCHAR_H)
		    wt = IS_CHAR_CLASS (str);
		    if (wt == 0)
		      /* Invalid character class name.  */
		      return FNM_NOMATCH;

# if defined _LIBC && ! WIDE_CHAR_VERSION
		    /* The following code is glibc specific but does
		       there a good job in speeding up the code since
		       we can avoid the btowc() call.  */
		    if (_ISCTYPE ((UCHAR) *n, wt))
		      goto matched;
# else
		    if (ISWCTYPE (BTOWC ((UCHAR) *n), wt))
		      goto matched;
# endif
#else
		    if ((STREQ (str, L("alnum")) && ISALNUM ((UCHAR) *n))
			|| (STREQ (str, L("alpha")) && ISALPHA ((UCHAR) *n))
			|| (STREQ (str, L("blank")) && ISBLANK ((UCHAR) *n))
			|| (STREQ (str, L("cntrl")) && ISCNTRL ((UCHAR) *n))
			|| (STREQ (str, L("digit")) && ISDIGIT ((UCHAR) *n))
			|| (STREQ (str, L("graph")) && ISGRAPH ((UCHAR) *n))
			|| (STREQ (str, L("lower")) && ISLOWER ((UCHAR) *n))
			|| (STREQ (str, L("print")) && ISPRINT ((UCHAR) *n))
			|| (STREQ (str, L("punct")) && ISPUNCT ((UCHAR) *n))
			|| (STREQ (str, L("space")) && ISSPACE ((UCHAR) *n))
			|| (STREQ (str, L("upper")) && ISUPPER ((UCHAR) *n))
			|| (STREQ (str, L("xdigit")) && ISXDIGIT ((UCHAR) *n)))
		      goto matched;
#endif
		    c = *p++;
		  }
#ifdef _LIBC
		else if (c == L('[') && *p == L('='))
		  {
		    UCHAR str[1];
		    uint32_t nrules =
		      _NL_CURRENT_WORD (LC_COLLATE, _NL_COLLATE_NRULES);
		    const CHAR *startp = p;

		    c = *++p;
		    if (c == L('\0'))
		      {
			p = startp;
			c = L('[');
			goto normal_bracket;
		      }
		    str[0] = c;

		    c = *++p;
		    if (c != L('=') || p[1] != L(']'))
		      {
			p = startp;
			c = L('[');
			goto normal_bracket;
		      }
		    p += 2;

		    if (nrules == 0)
		      {
			if ((UCHAR) *n == str[0])
			  goto matched;
		      }
		    else
		      {
			const int32_t *table;
# if WIDE_CHAR_VERSION
			const int32_t *weights;
			const int32_t *extra;
# else
			const unsigned char *weights;
			const unsigned char *extra;
# endif
			const int32_t *indirect;
			int32_t idx;
			const UCHAR *cp = (const UCHAR *) str;

			/* This #include defines a local function!  */
# if WIDE_CHAR_VERSION
#  include <locale/weightwc.h>
# else
#  include <locale/weight.h>
# endif

# if WIDE_CHAR_VERSION
			table = (const int32_t *)
			  _NL_CURRENT (LC_COLLATE, _NL_COLLATE_TABLEWC);
			weights = (const int32_t *)
			  _NL_CURRENT (LC_COLLATE, _NL_COLLATE_WEIGHTWC);
			extra = (const int32_t *)
			  _NL_CURRENT (LC_COLLATE, _NL_COLLATE_EXTRAWC);
			indirect = (const int32_t *)
			  _NL_CURRENT (LC_COLLATE, _NL_COLLATE_INDIRECTWC);
# else
			table = (const int32_t *)
			  _NL_CURRENT (LC_COLLATE, _NL_COLLATE_TABLEMB);
			weights = (const unsigned char *)
			  _NL_CURRENT (LC_COLLATE, _NL_COLLATE_WEIGHTMB);
			extra = (const unsigned char *)
			  _NL_CURRENT (LC_COLLATE, _NL_COLLATE_EXTRAMB);
			indirect = (const int32_t *)
			  _NL_CURRENT (LC_COLLATE, _NL_COLLATE_INDIRECTMB);
# endif

			idx = findidx (&cp);
			if (idx != 0)
			  {
			    /* We found a table entry.  Now see whether the
			       character we are currently at has the same
			       equivalance class value.  */
			    int len = weights[idx];
			    int32_t idx2;
			    const UCHAR *np = (const UCHAR *) n;

			    idx2 = findidx (&np);
			    if (idx2 != 0 && len == weights[idx2])
			      {
				int cnt = 0;

				while (cnt < len
				       && (weights[idx + 1 + cnt]
					   == weights[idx2 + 1 + cnt]))
				  ++cnt;

				if (cnt == len)
				  goto matched;
			      }
			  }
		      }

		    c = *p++;
		  }
#endif
		else if (c == L('\0'))
		  /* [ (unterminated) loses.  */
		  return FNM_NOMATCH;
		else
		  {
		    int is_range = 0;

#ifdef _LIBC
		    int is_seqval = 0;

		    if (c == L('[') && *p == L('.'))
		      {
			uint32_t nrules =
			  _NL_CURRENT_WORD (LC_COLLATE, _NL_COLLATE_NRULES);
			const CHAR *startp = p;
			size_t c1 = 0;

			while (1)
			  {
			    c = *++p;
			    if (c == L('.') && p[1] == L(']'))
			      {
				p += 2;
				break;
			      }
			    if (c == '\0')
			      return FNM_NOMATCH;
			    ++c1;
			  }

			/* We have to handling the symbols differently in
			   ranges since then the collation sequence is
			   important.  */
			is_range = *p == L('-') && p[1] != L('\0');

			if (nrules == 0)
			  {
			    /* There are no names defined in the collation
			       data.  Therefore we only accept the trivial
			       names consisting of the character itself.  */
			    if (c1 != 1)
			      return FNM_NOMATCH;

			    if (!is_range && *n == startp[1])
			      goto matched;

			    cold = startp[1];
			    c = *p++;
			  }
			else
			  {
			    int32_t table_size;
			    const int32_t *symb_table;
# ifdef WIDE_CHAR_VERSION
			    char str[c1];
			    unsigned int strcnt;
# else
#  define str (startp + 1)
# endif
			    const unsigned char *extra;
			    int32_t idx;
			    int32_t elem;
			    int32_t second;
			    int32_t hash;

# ifdef WIDE_CHAR_VERSION
			    /* We have to convert the name to a single-byte
			       string.  This is possible since the names
			       consist of ASCII characters and the internal
			       representation is UCS4.  */
			    for (strcnt = 0; strcnt < c1; ++strcnt)
			      str[strcnt] = startp[1 + strcnt];
#endif

			    table_size =
			      _NL_CURRENT_WORD (LC_COLLATE,
						_NL_COLLATE_SYMB_HASH_SIZEMB);
			    symb_table = (const int32_t *)
			      _NL_CURRENT (LC_COLLATE,
					   _NL_COLLATE_SYMB_TABLEMB);
			    extra = (const unsigned char *)
			      _NL_CURRENT (LC_COLLATE,
					   _NL_COLLATE_SYMB_EXTRAMB);

			    /* Locate the character in the hashing table.  */
			    hash = elem_hash (str, c1);

			    idx = 0;
			    elem = hash % table_size;
			    second = hash % (table_size - 2);
			    while (symb_table[2 * elem] != 0)
			      {
				/* First compare the hashing value.  */
				if (symb_table[2 * elem] == hash
				    && c1 == extra[symb_table[2 * elem + 1]]
				    && memcmp (str,
					       &extra[symb_table[2 * elem + 1]
						     + 1], c1) == 0)
				  {
				    /* Yep, this is the entry.  */
				    idx = symb_table[2 * elem + 1];
				    idx += 1 + extra[idx];
				    break;
				  }

				/* Next entry.  */
				elem += second;
			      }

			    if (symb_table[2 * elem] != 0)
			      {
				/* Compare the byte sequence but only if
				   this is not part of a range.  */
# ifdef WIDE_CHAR_VERSION
				int32_t *wextra;

				idx += 1 + extra[idx];
				/* Adjust for the alignment.  */
				idx = (idx + 3) & ~4;

				wextra = (int32_t *) &extra[idx + 4];
# endif

				if (! is_range)
				  {
# ifdef WIDE_CHAR_VERSION
				    for (c1 = 0; c1 < wextra[idx]; ++c1)
				      if (n[c1] != wextra[1 + c1])
					break;

				    if (c1 == wextra[idx])
				      goto matched;
# else
				    for (c1 = 0; c1 < extra[idx]; ++c1)
				      if (n[c1] != extra[1 + c1])
					break;

				    if (c1 == extra[idx])
				      goto matched;
# endif
				  }

				/* Get the collation sequence value.  */
				is_seqval = 1;
# ifdef WIDE_CHAR_VERSION
				cold = wextra[1 + wextra[idx]];
# else
				/* Adjust for the alignment.  */
				idx += 1 + extra[idx];
				idx = (idx + 3) & ~4;
				cold = *((int32_t *) &extra[idx]);
# endif

				c = *p++;
			      }
			    else if (c1 == 1)
			      {
				/* No valid character.  Match it as a
				   single byte.  */
				if (!is_range && *n == str[0])
				  goto matched;

				cold = str[0];
				c = *p++;
			      }
			    else
			      return FNM_NOMATCH;
			  }
		      }
		    else
# undef str
#endif
		      {
			c = FOLD (c);
		      normal_bracket:

			/* We have to handling the symbols differently in
			   ranges since then the collation sequence is
			   important.  */
			is_range = *p == L('-') && p[1] != L('\0');

			if (!is_range && c == fn)
			  goto matched;

			cold = c;
			c = *p++;
		      }

		    if (c == L('-') && *p != L(']'))
		      {
#if _LIBC
			/* We have to find the collation sequence
			   value for C.  Collation sequence is nothing
			   we can regularly access.  The sequence
			   value is defined by the order in which the
			   definitions of the collation values for the
			   various characters appear in the source
			   file.  A strange concept, nowhere
			   documented.  */
			uint32_t fcollseq;
			uint32_t lcollseq;
			UCHAR cend = *p++;

# ifdef WIDE_CHAR_VERSION
			/* Search in the `names' array for the characters.  */
			fcollseq = collseq_table_lookup (collseq, fn);
			if (fcollseq == ~((uint32_t) 0))
			  /* XXX We don't know anything about the character
			     we are supposed to match.  This means we are
			     failing.  */
			  goto range_not_matched;

			if (is_seqval)
			  lcollseq = cold;
			else
			  lcollseq = collseq_table_lookup (collseq, cold);
# else
			fcollseq = collseq[fn];
			lcollseq = is_seqval ? cold : collseq[(UCHAR) cold];
# endif

			is_seqval = 0;
			if (cend == L('[') && *p == L('.'))
			  {
			    uint32_t nrules =
			      _NL_CURRENT_WORD (LC_COLLATE,
						_NL_COLLATE_NRULES);
			    const CHAR *startp = p;
			    size_t c1 = 0;

			    while (1)
			      {
				c = *++p;
				if (c == L('.') && p[1] == L(']'))
				  {
				    p += 2;
				    break;
				  }
				if (c == '\0')
				  return FNM_NOMATCH;
				++c1;
			      }

			    if (nrules == 0)
			      {
				/* There are no names defined in the
				   collation data.  Therefore we only
				   accept the trivial names consisting
				   of the character itself.  */
				if (c1 != 1)
				  return FNM_NOMATCH;

				cend = startp[1];
			      }
			    else
			      {
				int32_t table_size;
				const int32_t *symb_table;
# ifdef WIDE_CHAR_VERSION
				char str[c1];
				unsigned int strcnt;
# else
#  define str (startp + 1)
# endif
				const unsigned char *extra;
				int32_t idx;
				int32_t elem;
				int32_t second;
				int32_t hash;

# ifdef WIDE_CHAR_VERSION
				/* We have to convert the name to a single-byte
				   string.  This is possible since the names
				   consist of ASCII characters and the internal
				   representation is UCS4.  */
				for (strcnt = 0; strcnt < c1; ++strcnt)
				  str[strcnt] = startp[1 + strcnt];
# endif

				table_size =
				  _NL_CURRENT_WORD (LC_COLLATE,
						    _NL_COLLATE_SYMB_HASH_SIZEMB);
				symb_table = (const int32_t *)
				  _NL_CURRENT (LC_COLLATE,
					       _NL_COLLATE_SYMB_TABLEMB);
				extra = (const unsigned char *)
				  _NL_CURRENT (LC_COLLATE,
					       _NL_COLLATE_SYMB_EXTRAMB);

				/* Locate the character in the hashing
                                   table.  */
				hash = elem_hash (str, c1);

				idx = 0;
				elem = hash % table_size;
				second = hash % (table_size - 2);
				while (symb_table[2 * elem] != 0)
				  {
				/* First compare the hashing value.  */
				    if (symb_table[2 * elem] == hash
					&& (c1
					    == extra[symb_table[2 * elem + 1]])
					&& memcmp (str,
						   &extra[symb_table[2 * elem + 1]
							 + 1], c1) == 0)
				      {
					/* Yep, this is the entry.  */
					idx = symb_table[2 * elem + 1];
					idx += 1 + extra[idx];
					break;
				      }

				    /* Next entry.  */
				    elem += second;
				  }

				if (symb_table[2 * elem] != 0)
				  {
				    /* Compare the byte sequence but only if
				       this is not part of a range.  */
# ifdef WIDE_CHAR_VERSION
				    int32_t *wextra;

				    idx += 1 + extra[idx];
				    /* Adjust for the alignment.  */
				    idx = (idx + 3) & ~4;

				    wextra = (int32_t *) &extra[idx + 4];
# endif
				    /* Get the collation sequence value.  */
				    is_seqval = 1;
# ifdef WIDE_CHAR_VERSION
				    cend = wextra[1 + wextra[idx]];
# else
				    /* Adjust for the alignment.  */
				    idx += 1 + extra[idx];
				    idx = (idx + 3) & ~4;
				    cend = *((int32_t *) &extra[idx]);
# endif
				  }
				else if (symb_table[2 * elem] != 0 && c1 == 1)
				  {
				    cend = str[0];
				    c = *p++;
				  }
				else
				  return FNM_NOMATCH;
			      }
# undef str
			  }
			else
			  {
			    if (!(flags & FNM_NOESCAPE) && cend == L('\\'))
			      cend = *p++;
			    if (cend == L('\0'))
			      return FNM_NOMATCH;
			    cend = FOLD (cend);
			  }

			/* XXX It is not entirely clear to me how to handle
			   characters which are not mentioned in the
			   collation specification.  */
			if (
# ifdef WIDE_CHAR_VERSION
			    lcollseq == 0xffffffff ||
# endif
			    lcollseq <= fcollseq)
			  {
			    /* We have to look at the upper bound.  */
			    uint32_t hcollseq;

			    if (is_seqval)
			      hcollseq = cend;
			    else
			      {
# ifdef WIDE_CHAR_VERSION
				hcollseq =
				  collseq_table_lookup (collseq, cend);
				if (hcollseq == ~((uint32_t) 0))
				  {
				    /* Hum, no information about the upper
				       bound.  The matching succeeds if the
				       lower bound is matched exactly.  */
				    if (lcollseq != fcollseq)
				      goto range_not_matched;

				    goto matched;
				  }
# else
				hcollseq = collseq[cend];
# endif
			      }

			    if (lcollseq <= hcollseq && fcollseq <= hcollseq)
			      goto matched;
			  }
# ifdef WIDE_CHAR_VERSION
		      range_not_matched:
# endif
#else
			/* We use a boring value comparison of the character
			   values.  This is better than comparing using
			   `strcoll' since the latter would have surprising
			   and sometimes fatal consequences.  */
			UCHAR cend = *p++;

			if (!(flags & FNM_NOESCAPE) && cend == L('\\'))
			  cend = *p++;
			if (cend == L('\0'))
			  return FNM_NOMATCH;

			/* It is a range.  */
			if (cold <= fn && fn <= c)
			  goto matched;
#endif

			c = *p++;
		      }
		  }

		if (c == L(']'))
		  break;
	      }

	    if (!not)
	      return FNM_NOMATCH;
	    break;

	  matched:
	    /* Skip the rest of the [...] that already matched.  */
	    do
	      {
	      ignore_next:
		c = *p++;

		if (c == L('\0'))
		  /* [... (unterminated) loses.  */
		  return FNM_NOMATCH;

		if (!(flags & FNM_NOESCAPE) && c == L('\\'))
		  {
		    if (*p == L('\0'))
		      return FNM_NOMATCH;
		    /* XXX 1003.2d11 is unclear if this is right.  */
		    ++p;
		  }
		else if (c == L('[') && *p == L(':'))
		  {
		    int c1 = 0;
		    const CHAR *startp = p;

		    while (1)
		      {
			c = *++p;
			if (++c1 == CHAR_CLASS_MAX_LENGTH)
			  return FNM_NOMATCH;

			if (*p == L(':') && p[1] == L(']'))
			  break;

			if (c < L('a') || c >= L('z'))
			  {
			    p = startp;
			    goto ignore_next;
			  }
		      }
		    p += 2;
		    c = *p++;
		  }
		else if (c == L('[') && *p == L('='))
		  {
		    c = *++p;
		    if (c == L('\0'))
		      return FNM_NOMATCH;
		    c = *++p;
		    if (c != L('=') || p[1] != L(']'))
		      return FNM_NOMATCH;
		    p += 2;
		    c = *p++;
		  }
		else if (c == L('[') && *p == L('.'))
		  {
		    ++p;
		    while (1)
		      {
			c = *++p;
			if (c == '\0')
			  return FNM_NOMATCH;

			if (*p == L('.') && p[1] == L(']'))
			  break;
		      }
		    p += 2;
		    c = *p++;
		  }
	      }
	    while (c != L(']'));
	    if (not)
	      return FNM_NOMATCH;
	  }
	  break;

	default:
	  if (c != FOLD ((UCHAR) *n))
	    return FNM_NOMATCH;
	}

      ++n;
    }

  if (*n == '\0')
    return 0;

  if ((flags & FNM_LEADING_DIR) && *n == L('/'))
    /* The FNM_LEADING_DIR flag says that "foo*" matches "foobar/frobozz".  */
    return 0;

  return FNM_NOMATCH;
}

#undef FOLD
#undef CHAR
#undef UCHAR
#undef FCT
#undef STRCHR
#undef STRCHRNUL
#undef STRCOLL
#undef L
#undef BTOWC
