/* C code produced by gperf version 2.5 (GNU C++ version) */
/* Command-line: gperf -acCgopt -k1,2,5,$ -N charmap_hash programs/charmap-kw.gperf  */
/* Copyright (C) 1995, 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper, <drepper@gnu.ai.mit.edu>.

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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <string.h>

#include "locfile-token.h"
struct keyword_t ;

#define TOTAL_KEYWORDS 14
#define MIN_WORD_LENGTH 3
#define MAX_WORD_LENGTH 14
#define MIN_HASH_VALUE 3
#define MAX_HASH_VALUE 25
/* maximum key range = 23, duplicates = 0 */

#ifdef __GNUC__
inline
#endif
static unsigned int
hash (register const char *str, register int len)
{
  static const unsigned char asso_values[] =
    {
     26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
     26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
     26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
     26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
     26, 26, 26, 26, 26, 26, 26, 26, 14, 10,
     15,  4, 26, 26, 26, 26, 26, 26, 26, 26,
     26, 26, 26, 26, 26, 26, 26,  0,  0,  0,
     26, 26,  0,  0, 26, 26, 26,  0,  0, 26,
      0, 26, 26, 26,  5, 26, 26,  0, 26, 26,
     26, 26, 26, 26, 26,  0, 26, 26,  0,  0,
     26,  0, 26,  0, 26, 26, 26, 26, 26,  0,
     15,  0,  0, 26,  0,  0, 26,  0, 26, 26,
      0, 26, 26, 26, 26, 26, 26, 26,
    };
  register int hval = len;

  switch (hval)
    {
      default:
      case 5:
        hval += asso_values[(int) str[4]];
      case 4:
      case 3:
      case 2:
        hval += asso_values[(int) str[1]];
      case 1:
        hval += asso_values[(int) str[0]];
        break;
    }
  return hval + asso_values[(int) str[len - 1]];
}

#ifdef __GNUC__
inline
#endif
const struct keyword_t *
charmap_hash (register const char *str, register int len)
{
  static const struct keyword_t wordlist[] =
    {
      {"",}, {"",}, {"",}, 
      {"END",              tok_end,             0},
      {"",}, 
      {"WIDTH",            tok_width,           0},
      {"",}, 
      {"CHARMAP",          tok_charmap,         0},
      {"",}, 
      {"g3esc",            tok_g3esc,           1},
      {"mb_cur_max",       tok_mb_cur_max,      1},
      {"escape_char",      tok_escape_char,     1},
      {"comment_char",     tok_comment_char,    1},
      {"code_set_name",    tok_code_set_name,   1},
      {"WIDTH_VARIABLE",   tok_width_variable,  0},
      {"g1esc",            tok_g1esc,           1},
      {"",}, {"",}, 
      {"WIDTH_DEFAULT",    tok_width_default,   0},
      {"g0esc",            tok_g0esc,           1},
      {"g2esc",            tok_g2esc,           1},
      {"",}, {"",}, {"",}, {"",}, 
      {"mb_cur_min",       tok_mb_cur_min,      1},
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = wordlist[key].name;

          if (*s == *str && !strncmp (str + 1, s + 1, len - 1))
            return &wordlist[key];
        }
    }
  return 0;
}
