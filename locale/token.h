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

#ifndef _TOKEN_H
#define _TOKEN_H 1

/* Define those keywords which does not correspond directly to any 
   item in a category.  Those already have values.  */

enum token
  {
    /* We must make sure that these values do not overlap with the
       other possible return values of the lexer.  */
    TOK_LAST_USED = _NL_NUM,

    /* General tokens.  */
    TOK_END, TOK_COMMENT_CHAR, TOK_COPY, TOK_ESCAPE_CHAR, TOK_FROM,
    TOK_ENDOFLINE, TOK_IDENT, TOK_STRING, TOK_ELLIPSIS, TOK_CHAR,
    TOK_ILL_CHAR, TOK_NUMBER, TOK_MINUS1,

    /* Tokens from the collate category.  */
    TOK_IGNORE, TOK_UNDEFINED, TOK_BACKWARD, TOK_FORWARD, TOK_POSITION,
    TOK_COLLATING_ELEMENT, TOK_COLLATING_SYMBOL, TOK_ORDER_END,
    TOK_ORDER_START,

    /* Tokens from the ctype category.  */
    TOK_TOLOWER, TOK_TOUPPER,
    /* The order here is important.  It must correspond to the bits
       used for indicating the class membership (ctype.h).  */
    TOK_UPPER, TOK_LOWER, TOK_ALPHA, TOK_DIGIT, TOK_XDIGIT, TOK_SPACE,
    TOK_PRINT, TOK_GRAPH, TOK_BLANK, TOK_CNTRL, TOK_PUNCT,
  };

/*
 * Local Variables:
 *  mode:c
 *  c-basic-offset:2
 * End:
 */
#endif /* token.h */
