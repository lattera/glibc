%{
/* Expression parsing for plural form selection.
   Copyright (C) 2000 Free Software Foundation, Inc.
   Written by Ulrich Drepper <drepper@cygnus.com>, 2000.

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

#include <stdarg.h>
#include <stdlib.h>
#include "gettext.h"
#include "gettextP.h"

#define YYLEX_PARAM	&((struct parse_args *) arg)->cp
#define YYPARSE_PARAM	arg
%}
%pure_parser
%expect 10

%union {
  unsigned long int num;
  struct expression *exp;
}

%{
/* Prototypes for local functions.  */
static struct expression *new_exp (enum operator op, int n, ...);
static int yylex (YYSTYPE *lval, const char **pexp);
static void yyerror (const char *str);
%}

%left '?'
%left '|'
%left '&'
%left '=', '!'
%left '+', '-'
%left '*', '/', '%'
%token <num> NUMBER
%type <exp> exp

%%

start:	  exp
	  {
	    ((struct parse_args *) arg)->res = $1;
	  }
	;

exp:	  exp '?' exp ':' exp
	  {
	    if (($$ = new_exp (qmop, 3, $1, $3, $5)) == NULL)
	      YYABORT
	  }
	| exp '|' exp
	  {
	    if (($$ = new_exp (lor, 2, $1, $3)) == NULL)
	      YYABORT
	  }
	| exp '&' exp
	  {
	    if (($$ = new_exp (land, 2, $1, $3)) == NULL)
	      YYABORT
	  }
	| exp '=' exp
	  {
	    if (($$ = new_exp (equal, 2, $1, $3)) == NULL)
	      YYABORT
	  }
	| exp '!' exp
	  {
	    if (($$ = new_exp (not_equal, 2, $1, $3)) == NULL)
	      YYABORT
	  }
	| exp '+' exp
	  {
	    if (($$ = new_exp (plus, 2, $1, $3)) == NULL)
	      YYABORT
	  }
	| exp '-' exp
	  {
	    if (($$ = new_exp (minus, 2, $1, $3)) == NULL)
	      YYABORT
	  }
	| exp '*' exp
	  {
	    if (($$ = new_exp (mult, 2, $1, $3)) == NULL)
	      YYABORT
	  }
	| exp '/' exp
	  {
	    if (($$ = new_exp (divide, 2, $1, $3)) == NULL)
	      YYABORT
	  }
	| exp '%' exp
	  {
	    if (($$ = new_exp (module, 2, $1, $3)) == NULL)
	      YYABORT
	  }
	| 'n'
	  {
	    if (($$ = new_exp (var, 0)) == NULL)
	      YYABORT
	  }
	| NUMBER
	  {
	    if (($$ = new_exp (num, 0)) == NULL)
	      YYABORT;
	    $$->val.num = $1
	  }
	| '(' exp ')'
	  {
	    $$ = $2
	  }
	;

%%

static struct expression *
new_exp (enum operator op, int n, ...)
{
  struct expression *newp = (struct expression *) calloc (1, sizeof (*newp));
  va_list va;

  va_start (va, n);

  if (newp == NULL)
    while (n-- > 0)
      __gettext_free_exp (va_arg (va, struct expression *));
  else
    {
      newp->operation = op;
      if (n > 0)
	{
	  newp->val.args3.bexp = va_arg (va, struct expression *);
	  newp->val.args3.tbranch = va_arg (va, struct expression *);

	  if (n > 2)
	    newp->val.args3.fbranch = va_arg (va, struct expression *);

	  if (newp->val.args3.bexp == NULL
	      || newp->val.args3.tbranch == NULL
	      || (n > 2 && newp->val.args3.fbranch == NULL))
	    {
	      __gettext_free_exp (newp);
	      newp = NULL;
	    }
	}
    }

  va_end (va);

  return newp;
}

void
internal_function
__gettext_free_exp (struct expression *exp)
{
  if (exp == NULL)
    return;

  /* Handle the recursive case.  */
  switch (exp->operation)
    {
    case qmop:
      __gettext_free_exp (exp->val.args3.fbranch);
      /* FALLTHROUGH */

    case mult:
    case divide:
    case module:
    case plus:
    case minus:
    case equal:
    case not_equal:
    case land:
    case lor:
      __gettext_free_exp (exp->val.args2.right);
      __gettext_free_exp (exp->val.args2.left);
      break;

    default:
      break;
    }

  free (exp);
}


static int
yylex (YYSTYPE *lval, const char **pexp)
{
  const char *exp = *pexp;
  int result;

  while (1)
    {
      if (exp[0] == '\\' && exp[1] == '\n')
	{
	  exp += 2;
	  continue;
	}

      if (exp[0] == '\0')
	{
	  *pexp = exp;
	  return YYEOF;
	}

      if (exp[0] != ' ' && exp[0] != '\t')
	break;

      ++exp;
    }

  result = *exp++;
  switch (result)
    {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      {
	unsigned long int n = exp[-1] - '0';
	while (exp[0] >= '0' && exp[0] <= '9')
	  {
	    n *= 10;
	    n += exp[0] - '0';
	    ++exp;
	  }
	lval->num = n;
	result = NUMBER;
      }
      break;

    case '=':
    case '!':
      if (exp[0] == '=')
	++exp;
      else
	result = YYERRCODE;
      break;

    case '&':
    case '|':
      if (exp[0] == result)
	++exp;
      else
	result = YYERRCODE;
      break;

    case 'n':
    case '*':
    case '/':
    case '%':
    case '+':
    case '-':
    case '?':
    case ':':
    case '(':
    case ')':
      /* Nothing, just return the character.  */
      break;

    case ';':
    case '\n':
    case '\0':
      /* Be safe and let the user call this function again.  */
      --exp;
      result = YYEOF;
      break;

    default:
      result = YYERRCODE;
#if YYDEBUG != 0
      --exp;
#endif
      break;
    }

  *pexp = exp;

  return result;
}


static void
yyerror (const char *str)
{
  /* Do nothing.  We don't print error messages here.  */
}
