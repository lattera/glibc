/* POSIX.2 wordexp implementation.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Tim Waugh <tim@cyberelk.demon.co.uk>.

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

#include <wordexp.h>
#include <signal.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <string.h>
#include <glob.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <paths.h>
#include <errno.h>
#include <sys/param.h>
#include <stdio.h>
#include <fnmatch.h>

/* Undefine the following line for the production version.  */
/* #define NDEBUG 1 */
#include <assert.h>

/*
 * This is a recursive-descent-style word expansion routine.
 */

/* Some forward declarations */
static int parse_dollars (char **word, size_t *word_length, size_t *max_length,
			  const char *words, size_t *offset, int flags,
			  wordexp_t *pwordexp)
     internal_function;
static int parse_backtick (char **word, size_t *word_length,
			   size_t *max_length, const char *words,
			   size_t *offset, int flags, wordexp_t *pwordexp)
     internal_function;
static int eval_expr (char *expr, int *result) internal_function;

/* The w_*() functions manipulate word lists. */

#define W_CHUNK	(100)

static inline char *
w_addchar (char *buffer, size_t *actlen, size_t *maxlen, char ch)
     /* (lengths exclude trailing zero) */
{
  /* Add a character to the buffer, allocating room for it if needed.
   */

  if (*actlen == *maxlen)
    {
      char *old_buffer = buffer;
      assert (buffer == NULL || *maxlen != 0);
      *maxlen += W_CHUNK;
      buffer = realloc (buffer, 1 + *maxlen);

      if (buffer == NULL)
	free (old_buffer);
    }

  if (buffer != NULL)
    {
      buffer[*actlen] = ch;
      buffer[++(*actlen)] = '\0';
    }

  return buffer;
}

static char *
w_addstr (char *buffer, size_t *actlen, size_t *maxlen, const char *str)
     /* (lengths exclude trailing zero) */
{
  /* Add a string to the buffer, allocating room for it if needed.
   */
  size_t len;

  assert (str != NULL); /* w_addstr only called from this file */
  len = strlen (str);

  if (*actlen + len > *maxlen)
    {
      char *old_buffer = buffer;
      assert (buffer == NULL || *maxlen != 0);
      *maxlen += MAX (2 * len, W_CHUNK);
      buffer = realloc (old_buffer, 1 + *maxlen);

      if (buffer == NULL)
	free (old_buffer);
    }

  if (buffer != NULL)
    {
      memcpy (&buffer[*actlen], str, len);
      *actlen += len;
      buffer[*actlen] = '\0';
    }

  return buffer;
}

static int
w_addword (wordexp_t *pwordexp, char *word)
{
  /* Add a word to the wordlist */
  size_t num_p;

  num_p = 2 + pwordexp->we_wordc + pwordexp->we_offs;
  pwordexp->we_wordv = realloc (pwordexp->we_wordv, sizeof (char *) * num_p);
  if (pwordexp->we_wordv != NULL)
    {
      pwordexp->we_wordv[pwordexp->we_wordc++] = word;
      pwordexp->we_wordv[pwordexp->we_wordc] = NULL;
      return 0;
    }

  return WRDE_NOSPACE;
}

/* The parse_*() functions should leave *offset being the offset in 'words'
 * to the last character processed.
 */

static int
internal_function
parse_backslash (char **word, size_t *word_length, size_t *max_length,
		 const char *words, size_t *offset)
{
  /* We are poised _at_ a backslash, not in quotes */

  switch (words[1 + *offset])
    {
    case 0:
      /* Backslash is last character of input words */
      return WRDE_SYNTAX;

    case '\n':
      (*offset)++;
      break;

    default:
      *word = w_addchar (*word, word_length, max_length, words[1 + *offset]);
      if (*word == NULL)
	return WRDE_NOSPACE;

      (*offset)++;
      break;
    }

  return 0;
}

static int
internal_function
parse_qtd_backslash (char **word, size_t *word_length, size_t *max_length,
		     const char *words, size_t *offset)
{
  /* We are poised _at_ a backslash, inside quotes */

  switch (words[1 + *offset])
    {
    case 0:
      /* Backslash is last character of input words */
      return WRDE_SYNTAX;

    case '\n':
      ++(*offset);
      break;

    case '$':
    case '`':
    case '"':
    case '\\':
      *word = w_addchar (*word, word_length, max_length, words[1 + *offset]);
      if (*word == NULL)
	return WRDE_NOSPACE;

      ++(*offset);
      break;

    default:
      *word = w_addchar (*word, word_length, max_length, words[*offset]);
      if (*word != NULL)
	*word = w_addchar (*word, word_length, max_length, words[1 + *offset]);

      if (*word == NULL)
	return WRDE_NOSPACE;

      ++(*offset);
      break;
    }

  return 0;
}

static int
internal_function
parse_tilde (char **word, size_t *word_length, size_t *max_length,
	     const char *words, size_t *offset, size_t wordc)
{
  /* We are poised _at_ a tilde */
  size_t i;

  if (*word_length != 0)
    {
      if (!((*word)[*word_length - 1] == '=' && wordc == 0))
	{
	  if (!((*word)[*word_length - 1] == ':' &&
		strchr (*word, '=') && wordc == 0))
	    {
	      *word = w_addchar (*word, word_length, max_length, '~');
	      return *word ? 0 : WRDE_NOSPACE;
	    }
	}
    }

  for (i = 1 + *offset; words[i]; i++)
    {
      if (words[i] == ':' || words[i] == '/' || words[i] == ' ' ||
	  words[i] == '\t' || words[i] == 0 )
	break;

      if (words[i] == '\\')
	{
	  *word = w_addchar (*word, word_length, max_length, '~');
	  return *word ? 0 : WRDE_NOSPACE;
	}
    }

  if (i == 1 + *offset)
    {
      /* Tilde appears on its own */
      uid_t uid;
      struct passwd pwd, *tpwd;
      int buflen = 1000;
      char* buffer = __alloca (buflen);
      int result;

      uid = getuid ();

      while ((result = __getpwuid_r (uid, &pwd, buffer, buflen, &tpwd)) != 0
	     && errno == ERANGE)
	{
	  buflen += 1000;
	  buffer = __alloca (buflen);
	}

      if (result == 0 && pwd.pw_dir != NULL)
	{
	  *word = w_addstr (*word, word_length, max_length, pwd.pw_dir);
	  if (*word == NULL)
	    return WRDE_NOSPACE;
	}
      else
	{
	  *word = w_addchar (*word, word_length, max_length, '~');
	  if (*word == NULL)
	    return WRDE_NOSPACE;
	}
    }
  else
    {
      /* Look up user name in database to get home directory */
      char *user = __strndup (&words[1 + *offset], i - *offset);
      struct passwd pwd, *tpwd;
      int buflen = 1000;
      char* buffer = __alloca (buflen);
      int result;

      while ((result = __getpwnam_r (user, &pwd, buffer, buflen, &tpwd)) != 0
	     && errno == ERANGE)
	{
	  buflen += 1000;
	  buffer = __alloca (buflen);
	}

      if (result == 0 && pwd.pw_dir)
	*word = w_addstr (*word, word_length, max_length, pwd.pw_dir);
      else
	{
	  /* (invalid login name) */
	  *word = w_addchar (*word, word_length, max_length, '~');
	  if (*word != NULL)
	    *word = w_addstr (*word, word_length, max_length, user);
	}

      *offset = i - 1;
    }
  return *word ? 0 : WRDE_NOSPACE;
}

static int
internal_function
parse_glob (char **word, size_t *word_length, size_t *max_length,
	    const char *words, size_t *offset, int flags,
	    wordexp_t *pwordexp, char *ifs)
{
  /* We are poised just after a '*' or a '{'. */
  int error;
  glob_t globbuf;
  int match;
  char *matching_word;

  for (; words[*offset]; (*offset)++)
    switch (words[*offset])
      {
      case ' ':
      case '\t':
	break;

      case '$':
	error = parse_dollars (word, word_length, max_length, words, offset,
			       flags, pwordexp);
	if (error)
	  return error;

	continue;

      default:
	if (ifs == NULL || strchr (ifs, words[*offset]) == NULL)
	  {
	    *word = w_addchar (*word, word_length, max_length, words[*offset]);
	    if (*word == NULL)
	      return WRDE_NOSPACE;

	    continue;
	  }

	break;
      }

  error = glob (*word, GLOB_NOCHECK, NULL, &globbuf);

  if (error != 0)
    {
      /* We can only run into memory problems.  */
      assert (error == GLOB_NOSPACE);

      return WRDE_NOSPACE;
    }

  if (ifs && !*ifs)
    {
      /* No field splitting allowed */
      *word_length = strlen (globbuf.gl_pathv[0]);
      *word = realloc (*word, 1 + *word_length);
      if (*word == NULL)
	goto no_space;

      strcpy (*word, globbuf.gl_pathv[0]);

      for (match = 1; match < globbuf.gl_pathc && *word != NULL; ++match)
	{
	  *word = w_addchar (*word, word_length, max_length, ' ');
	  if (*word != NULL)
	    *word = w_addstr (*word, word_length, max_length,
			      globbuf.gl_pathv[match]);
	}

      /* Re-parse white space on return */
      globfree (&globbuf);
      --(*offset);
      return *word ? 0 : WRDE_NOSPACE;
    }

  /* here ifs != "" */
  free (*word);
  *word = NULL;
  *word_length = 0;

  matching_word = malloc (1 + strlen (globbuf.gl_pathv[0]));
  if (matching_word == NULL)
    goto no_space;

  strcpy (matching_word, globbuf.gl_pathv[0]);
  if (w_addword (pwordexp, matching_word) == WRDE_NOSPACE)
    goto no_space;

  for (match = 1; match < globbuf.gl_pathc; ++match)
    {
      matching_word = __strdup (globbuf.gl_pathv[match]);
      if (matching_word == NULL)
	goto no_space;

      if (w_addword (pwordexp, matching_word) == WRDE_NOSPACE)
	goto no_space;
    }

  globfree (&globbuf);

  /* Re-parse white space on return */
  --(*offset);
  return 0;

no_space:
  globfree (&globbuf);
  return WRDE_NOSPACE;
}

static int
parse_squote (char **word, size_t *word_length, size_t *max_length,
	      const char *words, size_t *offset)
{
  /* We are poised just after a single quote */
  for (; words[*offset]; ++(*offset))
    {
      if (words[*offset] != '\'')
	{
	  *word = w_addchar (*word, word_length, max_length, words[*offset]);
	  if (*word == NULL)
	    return WRDE_NOSPACE;
	}
      else return 0;
    }

  /* Unterminated string */
  return WRDE_SYNTAX;
}

/* Functions to evaluate an arithmetic expression */
static int
internal_function
eval_expr_val (char **expr, int *result)
{
  int sgn = +1;
  char *digit;

  /* Skip white space */
  for (digit = *expr; digit && *digit && isspace (*digit); ++digit);

  switch (*digit)
    {
    case '(':

      /* Scan for closing paren */
      for (++digit; **expr && **expr != ')'; ++(*expr));

      /* Is there one? */
      if (!**expr)
	return WRDE_SYNTAX;

      *(*expr)++ = 0;

      if (eval_expr (digit, result))
	return WRDE_SYNTAX;

      return 0;

    case '+':	/* Positive value */
      ++digit;
      break;

    case '-':	/* Negative value */
      ++digit;
      sgn = -1;
      break;

    default:
      if (!isdigit (*digit))
	return WRDE_SYNTAX;
    }

  *result = 0;
  for (; *digit && isdigit (*digit); ++digit)
    *result = (*result * 10) + (*digit - '0');

  *expr = digit;
  *result *= sgn;
  return 0;
}

static int
internal_function
eval_expr_multdiv (char **expr, int *result)
{
  int arg;

  /* Read a Value */
  if (eval_expr_val (expr, result))
    return WRDE_SYNTAX;

  while (**expr)
    {
      /* Skip white space */
      for (; *expr && **expr && isspace (**expr); ++(*expr));

      if (**expr == '*')
	{
	  (*expr)++;
	  if ((eval_expr_val (expr, &arg)) != 0)
	    return WRDE_SYNTAX;

	  *result *= arg;
	}
      else if (**expr == '/')
	{
	  (*expr)++;
	  if ((eval_expr_val (expr, &arg)) != 0)
	    return WRDE_SYNTAX;

	  *result /= arg;
	}
      else break;
    }

  return 0;
}

static int
internal_function
eval_expr (char *expr, int *result)
{
  int arg;

  /* Read a Multdiv */
  if ((eval_expr_multdiv (&expr, result)) != 0)
    return WRDE_SYNTAX;

  while (*expr)
    {
      /* Skip white space */
      for (; expr && *expr && isspace (*expr); ++expr);

      if (*expr == '+')
	{
	  expr++;
	  if ((eval_expr_multdiv (&expr, &arg)) != 0)
	    return WRDE_SYNTAX;

	  *result += arg;
	}
      else if (*expr == '-')
	{
	  expr++;
	  if ((eval_expr_multdiv (&expr, &arg)) != 0)
	    return WRDE_SYNTAX;

	  *result -= arg;
	}
      else break;
    }

  return 0;
}

static int
internal_function
parse_arith (char **word, size_t *word_length, size_t *max_length,
	     const char *words, size_t *offset, int flags, int bracket)
{
  /* We are poised just after "$((" or "$[" */
  int error;
  int paren_depth = 1;
  size_t expr_length = 0;
  size_t expr_maxlen = 0;
  char *expr = NULL;

  for (; words[*offset]; ++(*offset))
    {
      switch (words[*offset])
	{
	case '$':
	  error = parse_dollars (&expr, &expr_length, &expr_maxlen,
				 words, offset, flags, NULL);
	  /* The NULL here is to tell parse_dollars not to
	   * split the fields.
	   */
	  if (error)
	    {
	      free (expr);
	      return error;
	    }
	  break;

	case '`':
	  (*offset)++;
	  error = parse_backtick (&expr, &expr_length, &expr_maxlen,
				  words, offset, flags, NULL);
	  /* The NULL here is to tell parse_backtick not to
	   * split the fields.
	   */
	  if (error)
	    {
	      free (expr);
	      return error;
	    }
	  break;

	case '\\':
	  error = parse_qtd_backslash (&expr, &expr_length, &expr_maxlen,
				       words, offset);
	  if (error)
	    {
	      free (expr);
	      return error;
	    }
	  /* I think that a backslash within an
	   * arithmetic expansion is bound to
	   * cause an error sooner or later anyway though.
	   */
	  break;

	case ')':
	  if (--paren_depth == 0)
	    {
	      char *result;
	      int numresult = 0;

	      if (bracket || words[1 + *offset] != ')')
		return WRDE_SYNTAX;

	      ++(*offset);

	      /* Go - evaluate. */
	      if (*expr &&
		  eval_expr (expr, &numresult) != 0)
		return WRDE_SYNTAX;

	      result = __alloca (100);
	      __snprintf (result, 100, "%d", numresult);
	      *word = w_addstr (*word, word_length, max_length, result);
	      free (expr);
	      return *word ? 0 : WRDE_NOSPACE;
	    }
	  expr = w_addchar (expr, &expr_length, &expr_maxlen, words[*offset]);
	  if (expr == NULL)
	    return WRDE_NOSPACE;

	  break;

	case ']':
	  if (bracket && paren_depth == 1)
	    {
	      char *result;
	      int numresult = 0;

	      /* Go - evaluate. */
	      if (*expr && eval_expr (expr, &numresult) != 0)
		return WRDE_SYNTAX;

	      result = __alloca (100);
	      __snprintf (result, 100, "%d", numresult);
	      *word = w_addstr (*word, word_length, max_length, result);
	      free (expr);
	      return *word ? 0 : WRDE_NOSPACE;
	    }

	  free (expr);
	  return WRDE_SYNTAX;

	case '\n':
	case ';':
	case '{':
	case '}':
	  free (expr);
	  return WRDE_BADCHAR;

	case '(':
	  ++paren_depth;
	default:
	  expr = w_addchar (expr, &expr_length, &expr_maxlen, words[*offset]);
	  if (expr == NULL)
	    return WRDE_NOSPACE;
	}
    }

  /* Premature end */
  free (expr);
  return WRDE_SYNTAX;
}

/* Function to execute a command and retrieve the results */
/* pwordexp contains NULL if field-splitting is forbidden */
static int
internal_function
exec_comm (char *comm, char **word, size_t *word_length, size_t *max_length,
	   int flags, wordexp_t *pwordexp)
{
  int fildes[2];
  int bufsize = 128;
  int buflen;
  int state = 0;
  int i;
  char *buffer;
  pid_t pid;
  /* 'state' is:
   *  0 until first non-(whitespace-ifs)
   *  1 after a non-ifs
   *  2 after non-(whitespace-ifs)
   */

  /* Don't fork() unless necessary */
  if (!comm || !*comm)
    return 0;

  if (pipe (fildes))
    /* Bad */
    return WRDE_NOSPACE;

  if ((pid = fork ()) < 0)
    {
      /* Bad */
      return WRDE_NOSPACE;
    }

  if (pid == 0)
    {
      /* Child */
      /* Redirect input and output */
      dup2 (fildes[1], 1);

      /* Close stderr if we have to */
      if ((flags & WRDE_SHOWERR) == 0)
	close (2);

      execl (_PATH_BSHELL, _PATH_BSHELL, "-c", comm, NULL);

      /* Bad. What now? */
      exit (1);
    }

  /* Parent */

  close (fildes[1]);
  buffer = __alloca (bufsize);

  if (!pwordexp)
    { /* Quoted - no field splitting */

      while (1)
	{
	  if ((buflen = read (fildes[0], buffer, bufsize)) < 1)
	    {
	      if (waitpid (pid, NULL, WNOHANG) == 0)
		continue;
	      if ((buflen = read (fildes[0], buffer, bufsize)) < 1)
		break;
	    }

	  for (i = 0; i < buflen; ++i)
	    {
	      *word = w_addchar (*word, word_length, max_length, buffer[i]);
	      if (*word == NULL)
		{
		  close (fildes[0]);
		  return WRDE_NOSPACE;
		}
	    }
	}

      close (fildes[0]);
      return 0;
    }

  /* Not quoted - split fields.
   * NB. This isn't done properly yet.
   */
  while (1)
    {
      if ((buflen = read (fildes[0], buffer, bufsize)) < 1)
	{
	  if (waitpid (pid, NULL, WNOHANG) == 0)
	    continue;
	  if ((read (fildes[0], buffer, bufsize)) < 1)
	    break;
	}

      for (i = 0; i < buflen; ++i)
	{
	  /* What if these aren't field separators? FIX */
	  if (buffer[i] == ' ' || buffer[i] == '\t' || buffer[i] == '\n')
	    {
	      if (state != 0)
		state = 2;
	      continue;
	    }

	  if (state == 2)
	    {
	      /* End of word */
	      if (w_addword (pwordexp, *word) == WRDE_NOSPACE)
		{
		  close (fildes[0]);
		  return WRDE_NOSPACE;
		}

	      *word = NULL;
	      *word_length = 0;
	    }

	  state = 1;
	  *word = w_addchar (*word, word_length, max_length, buffer[i]);
	  if (*word == NULL)
	    {
	      close (fildes[0]);
	      return WRDE_NOSPACE;
	    }
	}
    }

  close (fildes[0]);
  return 0;
}

static int
parse_comm (char **word, size_t *word_length, size_t *max_length,
	    const char *words, size_t *offset, int flags, wordexp_t *pwordexp)
{
  /* We are poised just after "$(" */
  int paren_depth = 1;
  int error;
  size_t comm_length = 0;
  size_t comm_maxlen = 0;
  char *comm = NULL;

  for (; words[*offset]; ++(*offset))
    {
      switch (words[*offset])
	{
	case ')':
	  if (--paren_depth == 0)
	    {
	      /* Go -- give script to the shell */
	      error = exec_comm (comm, word, word_length, max_length, flags,
				 pwordexp);
	      free (comm);
	      return error;
	    }

	  /* This is just part of the script */
	  comm = w_addchar (comm, &comm_length, &comm_maxlen, words[*offset]);
	  if (comm == NULL)
	    return WRDE_NOSPACE;

	  break;

	case '(':
	  paren_depth++;
	default:
	  comm = w_addchar (comm, &comm_length, &comm_maxlen, words[*offset]);
	  if (comm == NULL)
	    return WRDE_NOSPACE;

	  break;
	}
    }

  /* Premature end */
  free (comm);
  return WRDE_SYNTAX;
}

static int
internal_function
parse_param (char **word, size_t *word_length, size_t *max_length,
	     const char *words, size_t *offset, int flags, wordexp_t *pwordexp)
{
  /* We are poised just after "$" */
  enum remove_pattern_enum
  {
    RP_NONE = 0,
    RP_SHORT_LEFT,
    RP_LONG_LEFT,
    RP_SHORT_RIGHT,
    RP_LONG_RIGHT
  };
  size_t start = *offset;
  size_t env_length = 0;
  size_t env_maxlen = 0;
  size_t pat_length = 0;
  size_t pat_maxlen = 0;
  char *env = NULL;
  char *pattern = NULL;
  char *value;
  char action = '\0';
  enum remove_pattern_enum remove = RP_NONE;
  int colon_seen = 0;
  int depth = 0;
  int error;

  for (; words[*offset]; ++(*offset))
    {
      switch (words[*offset])
	{
	case '{':
	  ++depth;

	  if (action != '\0' || remove != RP_NONE)
	    {
	      pattern = w_addchar (pattern, &pat_length, &pat_maxlen,
				   words[*offset]);
	      if (pattern == NULL)
		goto no_space;

	      break;
	    }

	  if (*offset == start)
	    break;

	  /* Otherwise evaluate */
	  /* (and re-parse this character) */
	  --(*offset);
	  goto envsubst;

	case '}':
	  if (words[start] != '{')
	      --(*offset);

	  if (action != '\0' || remove != RP_NONE)
	    {
	      if (--depth)
		{
		  pattern = w_addchar (pattern, &pat_length, &pat_maxlen,
				       words[*offset]);
		  if (pattern == NULL)
		    goto no_space;

		  break;
		}
	    }

	  /* Evaluate */
	  goto envsubst;

	case '#':
	  /* At the start?  (ie. 'string length') */
	  if (*offset == start + 1)
	    /* FIXME: This isn't written yet! */
	    break;

	  if (words[start] != '{')
	    {
	      /* Evaluate */
	      /* (and re-parse this character) */
	      --(*offset);
	      goto envsubst;
	    }

	  /* Separating variable name from prefix pattern? */

	  if (remove == RP_NONE)
	    {
	      remove = RP_SHORT_LEFT;
	      break;
	    }
	  else if (remove == RP_SHORT_LEFT)
	    {
	      remove = RP_LONG_LEFT;
	      break;
	    }

	  /* Must be part of prefix/suffix pattern. */
	  pattern = w_addchar (pattern, &pat_length, &pat_maxlen,
			       words[*offset]);
	  if (pattern == NULL)
	    goto no_space;

	  break;

	case '%':
	  if (!*env)
	    goto syntax;

	  /* Separating variable name from suffix pattern? */
	  if (remove == RP_NONE)
	    {
	      remove = RP_SHORT_RIGHT;
	      break;
	    }
	  else if (remove == RP_SHORT_RIGHT)
	    {
	      remove = RP_LONG_RIGHT;
	      break;
	    }

	  /* Must be part of prefix/suffix pattern. */
	  pattern = w_addchar (pattern, &pat_length, &pat_maxlen,
			       words[*offset]);
	  if (pattern == NULL)
	    goto no_space;

	  break;

	case ':':
	  if (!*env)
	    goto syntax;

	  if (action != '\0' || remove != RP_NONE)
	    {
	      pattern = w_addchar (pattern, &pat_length, &pat_maxlen,
				   words[*offset]);
	      if (pattern == NULL)
		goto no_space;

	      break;
	    }

	  if ((words[1 + *offset] == '-') || (words[1 + *offset] == '=') ||
	      (words[1 + *offset] == '?') || (words[1 + *offset] == '+'))
	    {
	      colon_seen = 1;
	      break;
	    }

	  goto syntax;

	case '-':
	case '=':
	case '?':
	case '+':
	  if (!*env)
	    goto syntax;

	  if (action != '\0' || remove != RP_NONE)
	    {
	      pattern = w_addchar (pattern, &pat_length, &pat_maxlen,
				   words[*offset]);
	      if (pattern == NULL)
		goto no_space;

	      break;
	    }

	  action = words[*offset];
	  break;

	case '\\':
	  if (action != '\0' || remove != RP_NONE)
	    {
	      /* Um. Is this right? */
	      error = parse_qtd_backslash (word, word_length, max_length,
					   words, offset);
	      if (error == 0)
		break;
	    }
	  else
	    {
	      error = WRDE_SYNTAX;
	    }

	  if (env)
	    free (env);

	  if (pattern != NULL)
	    free (pattern);

	  return error;

	default:
	  if (action != '\0' || remove != RP_NONE)
	    {
	      pattern = w_addchar (pattern, &pat_length, &pat_maxlen,
				   words[*offset]);
	      if (pattern == NULL)
		goto no_space;

	      break;
	    }

	  if ((words[start] == '{') || isalpha (words[*offset]))
	    {
	      env = w_addchar (env, &env_length, &env_maxlen, words[*offset]);
	      if (env == NULL)
		goto no_space;

	      break;
	    }

	  --(*offset);
	  goto envsubst;
	}
    }

  /* End of input string */
  --(*offset);

envsubst:
  if (words[start] == '{' && words[*offset] != '}')
    goto syntax;

  if (!env || !*env)
    {
      *offset = start - 1;
      *word = w_addchar (*word, word_length, max_length, '$');
      free (env);
      free (pattern);
      return *word ? 0 : WRDE_NOSPACE;
    }

  value = getenv (env);

  if (action != '\0' || remove != RP_NONE)
    {
      switch (action)
	{
	case 0:
	  {
	    char *p;
	    char c;
	    char *end;

	    if (!pattern || !*pattern)
	      break;

	    end = value + strlen (value);

	    if (value == NULL)
	      break;

	    switch (remove)
	      {
	      case RP_SHORT_LEFT:
		for (p = value; p <= end; p++)
		  {
		    c = *p;
		    *p = '\0';
		    if (fnmatch (pattern, value, 0) != FNM_NOMATCH)
		      {
			*p = c;
			value = p;
			break;
		      }
		    *p = c;
		  }

		break;

	      case RP_LONG_LEFT:
		for (p = end; p >= value; p--)
		  {
		    c = *p;
		    *p = '\0';
		    if (fnmatch (pattern, value, 0) != FNM_NOMATCH)
		      {
			*p = c;
			value = p;
			break;
		      }
		    *p = c;
		  }

		break;

	      case RP_SHORT_RIGHT:
		for (p = end; p >= value; p--)
		  {
		    if (fnmatch (pattern, p, 0) != FNM_NOMATCH)
		      {
			*p = '\0';
			break;
		      }
		  }

		break;

	      case RP_LONG_RIGHT:
		for (p = value; p <= end; p++)
		  {
		    if (fnmatch (pattern, p, 0) != FNM_NOMATCH)
		      {
			*p = '\0';
			break;
		      }
		  }

		break;

	      default:
		assert (! "Unexpected `remove' value\n");
	      }

	    break;
	  }

	case '?':
	  if (value && *value)
	    break;

	  if (!colon_seen && value)
	    {
	      /* Substitute NULL */
	      free (env);
	      free (pattern);
	      return 0;
	    }

	  /* Error - exit */
	  fprintf (stderr, "%s: ", env);

	  if (*pattern)
	    {
	      /* Expand 'pattern' and write it to stderr */
	      wordexp_t	we;

	      error = wordexp (pattern, &we, flags);

	      if (error == 0)
		{
		  int i;

		  for (i = 0; i < we.we_wordc; ++i)
		    {
		      fprintf (stderr, "%s%s", i ? " " : "", we.we_wordv[i]);
		    }

		  fprintf (stderr, "\n");
		  error = WRDE_BADVAL;
		}

	      wordfree (&we);
	      free (env);
	      free (pattern);
	      return error;
	    }

	  fprintf (stderr, "parameter null or not set\n");
	  free (env);
	  free (pattern);
	  return WRDE_BADVAL;

	default:
	  printf ("warning: parameter substitution does not yet support \"%s%c\"\n", colon_seen?":":"", action);
	}
    }

  free (env);
  free (pattern);

  if (value == NULL)
    {
      /* Variable not defined */
      if (flags & WRDE_UNDEF)
	return WRDE_SYNTAX;

      return 0;
    }

  if (pwordexp == NULL)
    /* Quoted - no field split */
    *word = w_addstr (*word, word_length, max_length, value);
  else
    /* Should field-split here - FIX */
    *word = w_addstr (*word, word_length, max_length, value);

  return *word ? 0 : WRDE_NOSPACE;

no_space:
  if (env)
    free (env);

  if (pattern)
    free (pattern);

  return WRDE_NOSPACE;

syntax:
  if (env)
    free (env);

  if (pattern)
    free (pattern);

  return WRDE_SYNTAX;
}

static int
internal_function
parse_dollars (char **word, size_t *word_length, size_t *max_length,
	       const char *words, size_t *offset, int flags,
	       wordexp_t *pwordexp)
{
  /* We are poised _at_ "$" */
  switch (words[1 + *offset])
    {
    case '"':
    case '\'':
    case 0:
      *word = w_addchar (*word, word_length, max_length, '$');
      return *word ? 0 : WRDE_NOSPACE;

    case '(':
      if (words[2 + *offset] == '(')
	{
	  (*offset) += 3;
	  /* Call parse_arith -- 0 is for "no brackets" */
	  return parse_arith (word, word_length, max_length, words, offset,
			      flags, 0);
	}

      if (flags & WRDE_NOCMD)
	return WRDE_CMDSUB;

      (*offset) += 2;
      return parse_comm (word, word_length, max_length, words, offset, flags,
			 pwordexp);

    case '[':
      (*offset) += 2;
      /* Call parse_arith -- 1 is for "brackets" */
      return parse_arith (word, word_length, max_length, words, offset, flags,
			  1);

    case '{':
    default:
      ++(*offset);	/* parse_param needs to know if "{" is there */
      return parse_param (word, word_length, max_length, words, offset, flags,
			  pwordexp);
    }
}

static int
parse_backtick (char **word, size_t *word_length, size_t *max_length,
		const char *words, size_t *offset, int flags,
		wordexp_t *pwordexp)
{
  /* We are poised just after "`" */
  int error;
  size_t comm_length = 0;
  size_t comm_maxlen = 0;
  char *comm = NULL;
  int squoting = 0;

  for (; words[*offset]; ++(*offset))
    {
      switch (words[*offset])
	{
	case '`':
	  /* Go -- give the script to the shell */
	  error = exec_comm (comm, word, word_length, max_length, flags,
			     pwordexp);
	  free (comm);
	  return error;

	case '\\':
	  if (squoting)
	    {
	      error = parse_qtd_backslash (&comm, &comm_length, &comm_maxlen,
					   words, offset);

	      if (error)
		{
		  free (comm);
		  return error;
		}

	      break;
	    }

	  ++(*offset);
	  error = parse_backslash (&comm, &comm_length, &comm_maxlen, words,
				   offset);

	  if (error)
	    {
	      free (comm);
	      return error;
	    }

	  break;

	case '\'':
	  squoting = 1 - squoting;
	default:
	  comm = w_addchar (comm, &comm_length, &comm_maxlen, words[*offset]);
	  if (comm == NULL)
	    return WRDE_NOSPACE;
	}
    }

  /* Premature end */
  free (comm);
  return WRDE_SYNTAX;
}

static int
internal_function
parse_dquote (char **word, size_t *word_length, size_t *max_length,
	      const char *words, size_t *offset, int flags)
{
  /* We are poised just after a double-quote */
  int error;

  for (; words[*offset]; ++(*offset))
    {
      switch (words[*offset])
	{
	case '"':
	  return 0;

	case '$':
	  error = parse_dollars (word, word_length, max_length, words, offset,
				 flags, NULL);
	  /* The NULL here is to tell parse_dollars not to
	   * split the fields.
	   */
	  if (error)
	    return error;

	  break;

	case '`':
	  if (flags & WRDE_NOCMD)
	    return WRDE_CMDSUB;

	  ++(*offset);
	  error = parse_backtick (word, word_length, max_length, words,
				  offset, flags, NULL);
	  /* The NULL here is to tell parse_backtick not to
	   * split the fields.
	   */
	  if (error)
	    return error;

	  break;

	case '\\':
	  error = parse_qtd_backslash (word, word_length, max_length, words,
				       offset);

	  if (error)
	    return error;

	  break;

	default:
	  *word = w_addchar (*word, word_length, max_length, words[*offset]);
	  if (*word == NULL)
	    return WRDE_NOSPACE;
	}
    }

  /* Unterminated string */
  return WRDE_SYNTAX;
}

/*
 * wordfree() is to be called after pwordexp is finished with.
 */

void
wordfree (wordexp_t *pwordexp)
{

  /* wordexp can set pwordexp to NULL */
  if (pwordexp && pwordexp->we_wordv)
    {
      char **wordv = pwordexp->we_wordv;

      for (wordv += pwordexp->we_offs; *wordv; ++wordv)
	free (*wordv);

      free (pwordexp->we_wordv);
      pwordexp->we_wordv = NULL;
    }
}

/*
 * wordexp()
 */

int
wordexp (const char *words, wordexp_t *pwordexp, int flags)
{
  size_t wordv_offset;
  size_t words_offset;
  size_t word_length = 0;
  size_t max_length = 0;
  char *word = NULL;
  int error;
  char *ifs;
  char ifs_white[4];
  char **old_wordv = pwordexp->we_wordv;
  size_t old_wordc = pwordexp->we_wordc;

  if (flags & WRDE_REUSE)
    /* Minimal implementation of WRDE_REUSE for now */
    wordfree (pwordexp);

  if (flags & WRDE_DOOFFS)
    {
      pwordexp->we_wordv = calloc (1 + pwordexp->we_offs, sizeof (char *));
      if (pwordexp->we_wordv == NULL)
	return WRDE_NOSPACE;
    }
  else
    {
      pwordexp->we_wordv = calloc (1, sizeof (char *));
      if (pwordexp->we_wordv == NULL)
	return WRDE_NOSPACE;

      pwordexp->we_offs = 0;
    }

  if ((flags & WRDE_APPEND) == 0)
    pwordexp->we_wordc = 0;

  wordv_offset = pwordexp->we_offs + pwordexp->we_wordc;

  /* Find out what the field separators are.
   * There are two types: whitespace and non-whitespace.
   */
  ifs = getenv ("IFS");

  if (!ifs)
    ifs = strcpy (ifs_white, " \t\n");
  else
    {
      char *ifsch = ifs;
      char *whch = ifs_white;

      while (*ifsch != '\0')
	if ((*ifsch == ' ') || (*ifsch == '\t') || (*ifsch == '\n'))
	  {
	    /* White space IFS.  See first whether it is already in our
	       collection.  */
	    char *runp = ifs_white;

	    while (runp < whch && *runp != '\0' && *runp != *ifsch)
	      ++runp;

	    if (runp == whch)
	      *whch++ = *ifsch;
	  }
      *whch = '\0';
    }

  for (words_offset = 0 ; words[words_offset] ; ++words_offset)
    switch (words[words_offset])
      {
      case '\n':
      case '|':
      case '&':
      case ';':
      case '<':
      case '>':
      case '(':
      case ')':
      case '}':
	/* Fail */
	wordfree (pwordexp);
	pwordexp->we_wordc = 0;
	pwordexp->we_wordv = old_wordv;
	return WRDE_BADCHAR;

      case '\\':
	error = parse_backslash (&word, &word_length, &max_length, words,
				 &words_offset);

	if (error)
	  goto do_error;

	break;

      case '$':
	error = parse_dollars (&word, &word_length, &max_length, words,
			       &words_offset, flags, pwordexp);

	if (error)
	  goto do_error;

	break;

      case '`':
	if (flags & WRDE_NOCMD)
	  return WRDE_CMDSUB;

	++words_offset;
	error = parse_backtick (&word, &word_length, &max_length, words,
				&words_offset, flags, pwordexp);

	if (error)
	  goto do_error;

	break;

      case '"':
	++words_offset;
	error = parse_dquote (&word, &word_length, &max_length, words,
			      &words_offset, flags);

	if (error)
	  goto do_error;

	break;

      case '\'':
	++words_offset;
	error = parse_squote (&word, &word_length, &max_length, words,
			      &words_offset);

	if (error)
	  goto do_error;

	break;

      case '~':
	error = parse_tilde (&word, &word_length, &max_length, words,
			     &words_offset, pwordexp->we_wordc);

	if (error)
	  goto do_error;

	break;

      case '*':
      case '{':
	error = parse_glob (&word, &word_length, &max_length, words,
			    &words_offset, flags, pwordexp, ifs);

	if (error)
	  goto do_error;

	break;

      default:
	/* Is it a field separator? */
	if (strchr (ifs, words[words_offset]) == NULL)
	  {
	    /* "Ordinary" character -- add it to word */

	    word = w_addchar (word, &word_length, &max_length,
			      words[words_offset]);
	    if (word == NULL)
	      {
		error = WRDE_NOSPACE;
		goto do_error;
	      }

	    break;
	  }

	/* Field separator */
	if (strchr (ifs_white, words[words_offset]))
	  {
	    /* It's a whitespace IFS char.  Ignore it at the beginning
	       of a line and ignore multiple instances.  */
	    if (!word || !*word)
	      break;

	    if (w_addword (pwordexp, word) == WRDE_NOSPACE)
	      {
		error = WRDE_NOSPACE;
		goto do_error;
	      }

	    word = NULL;
	    word_length = 0;
	    break;
	  }

	/* It's a non-whitespace IFS char */

	/* Multiple non-whitespace IFS chars are treated as one;
	 * IS THIS CORRECT?
	 */
	if (word != NULL)
	  {
	    if (w_addword (pwordexp, word) == WRDE_NOSPACE)
	      {
		error = WRDE_NOSPACE;
		goto do_error;
	      }
	  }

	word = NULL;
	word_length = 0;
	max_length = 0;
      }

  /* End of string */

  /* There was a field separator at the end */
  if (word == NULL)
    return 0;

  /* There was no field separator at the end */
  return w_addword (pwordexp, word);

do_error:
  /* Error:
	free memory used, set we_wordc and wd_wordv back to what they were.
   */
  if (word != NULL)
    free (word);

  wordfree (pwordexp);
  pwordexp->we_wordv = old_wordv;
  pwordexp->we_wordc = old_wordc;
  return error;
}
