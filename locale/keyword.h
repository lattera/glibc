/* C code produced by gperf version 2.5 (GNU C++ version) */
/* Command-line: gperf -acCgopt -k1,2,5, keyword.gperf  */
/* `strncmp' is used for comparison.  */
#include <string.h>

/* This file defines `enum token'.  */
#include "token.h"
struct locale_keyword { char *name; enum token token_id; };

#define TOTAL_KEYWORDS 68
#define MIN_WORD_LENGTH 3
#define MAX_WORD_LENGTH 17
#define MIN_HASH_VALUE 4
#define MAX_HASH_VALUE 140
/* maximum key range = 137, duplicates = 0 */

#ifdef __GNUC__
inline
#endif
static unsigned int
hash (register const char *str, register int len)
{
  static const unsigned char asso_values[] =
    {
     141, 141, 141, 141, 141, 141, 141, 141, 141, 141,
     141, 141, 141, 141, 141, 141, 141, 141, 141, 141,
     141, 141, 141, 141, 141, 141, 141, 141, 141, 141,
     141, 141, 141, 141, 141, 141, 141, 141, 141, 141,
     141, 141, 141, 141, 141, 141, 141, 141, 141, 141,
     141, 141, 141, 141, 141, 141, 141, 141, 141, 141,
     141, 141, 141, 141, 141, 141, 141,   0, 141,  65,
       5,   0, 141,  30, 141, 141,   0, 141,   0,  95,
     141, 141,   0, 141,  45,  10, 141, 141, 141, 141,
     141, 141, 141, 141, 141,   5, 141,  10,  85,   0,
      20,   0,  40,  35,  30,  10, 141,   0,  30,  15,
      15,   0,   0, 141,  55,   0,   0,  80, 141,  15,
      10,   0, 141, 141, 141, 141, 141, 141,
    };
  register int hval = len;

  switch (hval)
    {
      default:
      case 5:
        hval += asso_values[str[4]];
      case 4:
      case 3:
      case 2:
        hval += asso_values[str[1]];
      case 1:
        hval += asso_values[str[0]];
    }
  return hval;
}

#ifdef __GNUC__
inline
#endif
const struct locale_keyword *
in_word_set (register const char *str, register int len)
{
  static const struct locale_keyword wordlist[] =
    {
      {"",}, {"",}, {"",}, {"",}, 
      {"copy",               TOK_COPY},
      {"space",              TOK_SPACE},
      {"yesstr",             YESSTR},
      {"toupper",            TOK_TOUPPER},
      {"position",           TOK_POSITION},
      {"",}, 
      {"t_fmt",              T_FMT},
      {"escape_char",        TOK_ESCAPE_CHAR},
      {"comment_char",       TOK_COMMENT_CHAR},
      {"positive_sign",      POSITIVE_SIGN},
      {"",}, 
      {"t_fmt_ampm",         T_FMT_AMPM},
      {"",}, 
      {"yesexpr",            YESEXPR},
      {"mon",                MON_1},
      {"p_sep_by_space",     P_SEP_BY_SPACE},
      {"LC_NUMERIC",         _NL_NUM_LC_NUMERIC},
      {"noexpr",             NOEXPR},
      {"tolower",            TOK_TOLOWER},
      {"p_cs_precedes",      P_CS_PRECEDES},
      {"UNDEFINED",          TOK_UNDEFINED},
      {"",}, 
      {"collating_symbol",   TOK_COLLATING_SYMBOL},
      {"collating_element",  TOK_COLLATING_ELEMENT},
      {"negative_sign",      NEGATIVE_SIGN},
      {"",}, 
      {"d_fmt",              D_FMT},
      {"",}, 
      {"mon_thousands_sep",  MON_THOUSANDS_SEP},
      {"day",                DAY_1},
      {"n_sep_by_space",     N_SEP_BY_SPACE},
      {"digit",              TOK_DIGIT},
      {"IGNORE",             TOK_IGNORE},
      {"LC_TIME",            _NL_NUM_LC_TIME},
      {"n_cs_precedes",      N_CS_PRECEDES},
      {"",}, 
      {"int_curr_symbol",    INT_CURR_SYMBOL},
      {"",}, {"",}, 
      {"thousands_sep",      THOUSANDS_SEP},
      {"",}, 
      {"am_pm",              AM_STR},
      {"xdigit",             TOK_XDIGIT},
      {"",}, 
      {"decimal_point",      DECIMAL_POINT},
      {"",}, 
      {"cntrl",              TOK_CNTRL},
      {"p_sign_posn",        P_SIGN_POSN},
      {"mon_decimal_point",  MON_DECIMAL_POINT},
      {"LC_CTYPE",           _NL_NUM_LC_CTYPE},
      {"",}, 
      {"alpha",              TOK_ALPHA},
      {"",}, 
      {"forward",            TOK_FORWARD},
      {"era",                ERA},
      {"",}, 
      {"print",              TOK_PRINT},
      {"",}, 
      {"mon_grouping",       MON_GROUPING},
      {"era_year",           ERA_YEAR},
      {"",}, {"",}, 
      {"n_sign_posn",        N_SIGN_POSN},
      {"",}, 
      {"END",                TOK_END},
      {"",}, 
      {"alt_digits",         ALT_DIGITS},
      {"",}, 
      {"d_t_fmt",            D_T_FMT},
      {"",}, {"",}, 
      {"nostr",              NOSTR},
      {"LC_MESSAGES",        _NL_NUM_LC_MESSAGES},
      {"",}, {"",}, {"",}, 
      {"int_frac_digits",    INT_FRAC_DIGITS},
      {"",}, {"",}, {"",}, 
      {"era_d_fmt",          ERA_D_FMT},
      {"punct",              TOK_PUNCT},
      {"",}, {"",}, {"",}, {"",}, 
      {"lower",              TOK_LOWER},
      {"",}, {"",}, {"",}, {"",}, 
      {"currency_symbol",    CURRENCY_SYMBOL},
      {"",}, {"",}, 
      {"grouping",           GROUPING},
      {"from",               TOK_FROM},
      {"abday",              ABDAY_1},
      {"",}, {"",}, {"",}, {"",}, 
      {"LC_COLLATE",         _NL_NUM_LC_COLLATE},
      {"LC_MONETARY",        _NL_NUM_LC_MONETARY},
      {"",}, {"",}, {"",}, {"",}, 
      {"frac_digits",        FRAC_DIGITS},
      {"",}, {"",}, {"",}, 
      {"abmon",              ABMON_1},
      {"",}, {"",}, 
      {"backward",           TOK_BACKWARD},
      {"order_end",          TOK_ORDER_END},
      {"blank",              TOK_BLANK},
      {"order_start",        TOK_ORDER_START},
      {"",}, {"",}, {"",}, 
      {"graph",              TOK_GRAPH},
      {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, {"",}, 
      {"",}, {"",}, {"",}, {"",}, {"",}, 
      {"upper",              TOK_UPPER},
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
