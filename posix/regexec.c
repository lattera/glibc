/* Extended regular expression matching and search library.
   Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Isamu Hasegawa <isamu@yamato.ibm.com>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#ifdef _LIBC
# ifndef _RE_DEFINE_LOCALE_FUNCTIONS
#  define _RE_DEFINE_LOCALE_FUNCTIONS 1
#  include <locale/localeinfo.h>
#  include <locale/elem-hash.h>
#  include <locale/coll-lookup.h>
# endif
#endif

#include "regex.h"
#include "regex_internal.h"

static void match_ctx_init (re_match_context_t *cache, int eflags, int n);
static void match_ctx_free (re_match_context_t *cache);
static void match_ctx_add_entry (re_match_context_t *cache, int node, int from,
                                 int to);
static int re_search_internal (const regex_t *preg, const char *string,
                               int length, int start, int range, size_t nmatch,
                               regmatch_t pmatch[], int eflags);
static inline re_dfastate_t *acquire_init_state_context (const regex_t *preg,
                                const re_string_t *input, int idx, int eflags);
static int check_matching (const regex_t *preg, re_string_t *input,
                           re_match_context_t *mctx, re_dfastate_t **state_log,
                           int start_idx, int fl_search, int fl_longest_match);
static int check_halt_node_context (const re_dfa_t *dfa, int node,
                                    unsigned int context);
static int check_halt_state_context (const regex_t *preg,
                                     const re_dfastate_t *state,
                                     const re_string_t *input, int idx,
                                     int eflags);
static int proceed_next_node (const regex_t *preg,
                              re_dfastate_t **state_log,
                              const re_match_context_t *mctx,
                              const re_string_t *input,
                              int *pidx, int node, re_node_set *eps_via_nodes);
static void set_regs (const regex_t *preg, re_dfastate_t **state_log,
                      const re_match_context_t *mctx, const re_string_t *input,
                      size_t nmatch, regmatch_t *pmatch, int last);
static int sift_states_iter_mb (const regex_t *preg, re_dfastate_t **state_log,
                                const re_match_context_t *mctx,
                                const re_string_t *input, int node_idx,
                                int str_idx, int max_str_idx);
static int sift_states_iter_bkref (const re_dfa_t *dfa,
                                   re_dfastate_t **state_log,
                                   struct re_backref_cache_entry *mctx_entry,
                                   int node_idx, int idx, int match_first,
                                   int match_last);
static void sift_states_backward (const regex_t *preg,
                                  re_dfastate_t **state_log,
                                  const re_match_context_t *mctx,
                                  const re_string_t *input, int last_node);
static void add_epsilon_backreference (const re_dfa_t *dfa,
                                       const re_match_context_t *mctx,
                                       const re_node_set *plog, int idx,
                                       re_node_set *state_buf);
static re_dfastate_t *transit_state (const regex_t *preg, re_dfastate_t *state,
                                     re_string_t *input, int fl_search,
                                     re_dfastate_t **state_log,
                                     re_match_context_t *mctx);
static re_dfastate_t *transit_state_sb (const regex_t *preg,
                                        re_dfastate_t *pstate,
                                        re_string_t *input, int fl_search,
                                        re_match_context_t *mctx);
static void transit_state_mb (const regex_t *preg, re_dfastate_t *pstate,
                              const re_string_t *input,
                              re_dfastate_t **state_log,
                              re_match_context_t *mctx);
static void transit_state_bkref (const regex_t *preg, re_dfastate_t *pstate,
                                 const re_string_t *input,
                                 re_dfastate_t **state_log,
                                 re_match_context_t *mctx);
static void transit_state_bkref_loop (const regex_t *preg,
                                      const re_string_t *input,
                                      re_node_set *nodes,
                                      re_dfastate_t **work_state_log,
                                      re_dfastate_t **state_log,
                                      re_match_context_t *mctx);
static re_dfastate_t **build_trtable (const regex_t *dfa,
                                      const re_dfastate_t *state,
                                      int fl_search);
static int check_node_accept_bytes (const regex_t *preg, int node_idx,
                                    const re_string_t *input, int idx);
static unsigned int find_collation_sequence_value (const unsigned char *mbs,
                                                   size_t name_len);
static int group_nodes_into_DFAstates (const regex_t *dfa,
                                       const re_dfastate_t *state,
                                       re_node_set *states_node,
                                       bitset *states_ch);
static int check_node_accept (const regex_t *preg, const re_token_t *node,
                              const re_string_t *input, int idx, int eflags);

/* Entry point for POSIX code.  */

/* regexec searches for a given pattern, specified by PREG, in the
   string STRING.

   If NMATCH is zero or REG_NOSUB was set in the cflags argument to
   `regcomp', we ignore PMATCH.  Otherwise, we assume PMATCH has at
   least NMATCH elements, and we set them to the offsets of the
   corresponding matched substrings.

   EFLAGS specifies `execution flags' which affect matching: if
   REG_NOTBOL is set, then ^ does not match at the beginning of the
   string; if REG_NOTEOL is set, then $ does not match at the end.

   We return 0 if we find a match and REG_NOMATCH if not.  */

int
regexec (preg, string, nmatch, pmatch, eflags)
    const regex_t *preg;
    const char *string;
    size_t nmatch;
    regmatch_t pmatch[];
    int eflags;
{
  int length = strlen (string);
  if (preg->no_sub)
    return re_search_internal (preg, string, length, 0, length, 0,
                               NULL, eflags);
  else
    return re_search_internal (preg, string, length, 0, length, nmatch,
                               pmatch, eflags);
}
#ifdef _LIBC
weak_alias (__regexec, regexec)
#endif

/* Entry points for GNU code.  */

/* re_match is like re_match_2 except it takes only a single string.  */

int
re_match (buffer, string, length, start, regs)
    struct re_pattern_buffer *buffer;
    const char *string;
    int length, start;
    struct re_registers *regs;
{
  int i, nregs, result, rval, eflags = 0;
  regmatch_t *pmatch;

  eflags |= (buffer->not_bol) ? REG_NOTBOL : 0;
  eflags |= (buffer->not_eol) ? REG_NOTEOL : 0;

  /* We need at least 1 register.  */
  nregs = ((regs == NULL) ? 1
           : ((regs->num_regs > buffer->re_nsub) ? buffer->re_nsub + 1
              : regs->num_regs + 1));
  pmatch = re_malloc (regmatch_t, nregs);
  if (pmatch == NULL)
    return -2;
  result = re_search_internal (buffer, string, length, start, 0,
                               nregs, pmatch, eflags);

  /* If caller wants register contents data back, do it.  */
  if (regs && !buffer->no_sub)
    {
      /* Have the register data arrays been allocated?  */
      if (buffer->regs_allocated == REGS_UNALLOCATED)
        { /* No.  So allocate them with malloc.  We need one
             extra element beyond `num_regs' for the `-1' marker
             GNU code uses.  */
          regs->num_regs = ((RE_NREGS > buffer->re_nsub + 1) ? RE_NREGS
                            : buffer->re_nsub + 1);
          regs->start = re_malloc (regoff_t, regs->num_regs);
          regs->end = re_malloc (regoff_t, regs->num_regs);
          if (regs->start == NULL || regs->end == NULL)
            {
              re_free (pmatch);
              return -2;
            }
          buffer->regs_allocated = REGS_REALLOCATE;
        }
      else if (buffer->regs_allocated == REGS_REALLOCATE)
        { /* Yes.  If we need more elements than were already
             allocated, reallocate them.  If we need fewer, just
             leave it alone.  */
          if (regs->num_regs < buffer->re_nsub + 1)
            {
              regs->num_regs = buffer->re_nsub + 1;
              regs->start = re_realloc (regs->start, regoff_t, regs->num_regs);
              regs->end = re_realloc (regs->end, regoff_t, regs->num_regs);
              if (regs->start == NULL || regs->end == NULL)
                {
                  re_free (pmatch);
                  return -2;
                }
            }
        }
      else
        {
          /* These braces fend off a "empty body in an else-statement"
             warning under GCC when assert expands to nothing.  */
          assert (buffer->regs_allocated == REGS_FIXED);
        }
    }

  /* Restore registers.  */
  if (regs != NULL)
    {
      for (i = 0; i <= nregs; ++i)
        {
          regs->start[i] = pmatch[i].rm_so;
          regs->end[i] = pmatch[i].rm_eo;
        }
      for ( ; i < regs->num_regs; ++i)
        {
          regs->start[i] = -1;
          regs->end[i] = -1;
        }
    }
  /* Return value is -1 if not match, the length of mathing otherwise.  */
  rval = (result) ? -1 : pmatch[0].rm_eo - pmatch[0].rm_so;
  re_free (pmatch);
  return rval;
}
#ifdef _LIBC
weak_alias (__re_match, re_match)
#endif

/* re_match_2 matches the compiled pattern in BUFP against the
   the (virtual) concatenation of STRING1 and STRING2 (of length SIZE1
   and SIZE2, respectively).  We start matching at POS, and stop
   matching at STOP.

   If REGS is non-null and the `no_sub' field of BUFP is nonzero, we
   store offsets for the substring each group matched in REGS.  See the
   documentation for exactly how many groups we fill.

   We return -1 if no match, -2 if an internal error.
   Otherwise, we return the length of the matched substring.  */

int
re_match_2 (buffer, string1, length1, string2, length2, start, regs, stop)
     struct re_pattern_buffer *buffer;
     const char *string1, *string2;
     int length1, length2, start, stop;
     struct re_registers *regs;
{
  int len, ret;
  char *str = re_malloc (char, length1 + length2);
  if (str == NULL)
    return -2;
  memcpy (str, string1, length1);
  memcpy (str + length1, string2, length2);
  len = (length1 + length2 < stop) ? length1 + length2 : stop;
  ret = re_match (buffer, str, len, start, regs);
  re_free (str);
  return ret;
}
#ifdef _LIBC
weak_alias (__re_match_2, re_match_2)
#endif

/* Like re_search_2, below, but only one string is specified, and
   doesn't let you say where to stop matching.  */

int
re_search (bufp, string, size, startpos, range, regs)
     struct re_pattern_buffer *bufp;
     const char *string;
     int size, startpos, range;
     struct re_registers *regs;
{
  int i, nregs, result, real_range, rval, eflags = 0;
  regmatch_t *pmatch;

  eflags |= (bufp->not_bol) ? REG_NOTBOL : 0;
  eflags |= (bufp->not_eol) ? REG_NOTEOL : 0;

  /* Check for out-of-range.  */
  if (startpos < 0 || startpos > size)
    return -1;

  /* We need at least 1 register.  */
  nregs = ((regs == NULL) ? 1
           : ((regs->num_regs > bufp->re_nsub) ? bufp->re_nsub + 1
              : regs->num_regs + 1));
  pmatch = re_malloc (regmatch_t, nregs);

  /* Correct range if we need.  */
  real_range = ((startpos + range > size) ? size - startpos
                : ((startpos + range < 0) ? -startpos : range));

  /* Compile fastmap if we haven't yet.  */
  if (bufp->fastmap != NULL && !bufp->fastmap_accurate)
    re_compile_fastmap (bufp);

  result = re_search_internal (bufp, string, size, startpos, real_range,
                               nregs, pmatch, eflags);

  /* If caller wants register contents data back, do it.  */
  if (regs && !bufp->no_sub)
    {
      /* Have the register data arrays been allocated?  */
      if (bufp->regs_allocated == REGS_UNALLOCATED)
        { /* No.  So allocate them with malloc.  We need one
             extra element beyond `num_regs' for the `-1' marker
             GNU code uses.  */
          regs->num_regs = ((RE_NREGS > bufp->re_nsub + 1) ? RE_NREGS
                            : bufp->re_nsub + 1);
          regs->start = re_malloc (regoff_t, regs->num_regs);
          regs->end = re_malloc (regoff_t, regs->num_regs);
          if (regs->start == NULL || regs->end == NULL)
            {
              re_free (pmatch);
              return -2;
            }
          bufp->regs_allocated = REGS_REALLOCATE;
        }
      else if (bufp->regs_allocated == REGS_REALLOCATE)
        { /* Yes.  If we need more elements than were already
             allocated, reallocate them.  If we need fewer, just
             leave it alone.  */
          if (regs->num_regs < bufp->re_nsub + 1)
            {
              regs->num_regs = bufp->re_nsub + 1;
              regs->start = re_realloc (regs->start, regoff_t, regs->num_regs);
              regs->end = re_realloc (regs->end, regoff_t, regs->num_regs);
              if (regs->start == NULL || regs->end == NULL)
                {
                  re_free (pmatch);
                  return -2;
                }
            }
        }
      else
        {
          /* These braces fend off a "empty body in an else-statement"
             warning under GCC when assert expands to nothing.  */
          assert (bufp->regs_allocated == REGS_FIXED);
        }
    }

  /* Restore registers.  */
  if (regs != NULL)
    {
      for (i = 0; i <= bufp->re_nsub; ++i)
        {
          regs->start[i] = pmatch[i].rm_so;
          regs->end[i] = pmatch[i].rm_eo;
        }
      for ( ; i < regs->num_regs; ++i)
        {
          regs->start[i] = -1;
          regs->end[i] = -1;
        }
    }
  /* Return value is -1 if not match, the position where the mathing starts
     otherwise.  */
  rval = (result) ? -1 : pmatch[0].rm_so;
  re_free (pmatch);
  return rval;
}
#ifdef _LIBC
weak_alias (__re_search, re_search)
#endif

/* Using the compiled pattern in BUFP, first tries to match the virtual
   concatenation of STRING1 and STRING2, starting first at index
   STARTPOS, then at STARTPOS + 1, and so on.

   STRING1 and STRING2 have length SIZE1 and SIZE2, respectively.

   RANGE is how far to scan while trying to match.  RANGE = 0 means try
   only at STARTPOS; in general, the last start tried is STARTPOS +
   RANGE.

   In REGS, return the indices of the virtual concatenation of STRING1
   and STRING2 that matched the entire BUFP->buffer and its contained
   subexpressions.

   Do not consider matching one past the index STOP in the virtual
   concatenation of STRING1 and STRING2.

   We return either the position in the strings at which the match was
   found, -1 if no match, or -2 if error.  */

int
re_search_2 (bufp, string1, length1, string2, length2, start, range, regs,
             stop)
    struct re_pattern_buffer *bufp;
    const char *string1, *string2;
    int length1, length2, start, range, stop;
    struct re_registers *regs;
{
  int len, ret;
  char *str = re_malloc (char, length1 + length2);
  memcpy (str, string1, length1);
  memcpy (str + length1, string2, length2);
  len = (length1 + length2 < stop) ? length1 + length2 : stop;
  ret = re_search (bufp, str, len, start, range, regs);
  re_free (str);
  return ret;
}
#ifdef _LIBC
weak_alias (__re_search_2, re_search_2)
#endif

/* Set REGS to hold NUM_REGS registers, storing them in STARTS and
   ENDS.  Subsequent matches using PATTERN_BUFFER and REGS will use
   this memory for recording register information.  STARTS and ENDS
   must be allocated using the malloc library routine, and must each
   be at least NUM_REGS * sizeof (regoff_t) bytes long.

   If NUM_REGS == 0, then subsequent matches should allocate their own
   register data.

   Unless this function is called, the first search or match using
   PATTERN_BUFFER will allocate its own register data, without
   freeing the old data.  */

void
re_set_registers (bufp, regs, num_regs, starts, ends)
    struct re_pattern_buffer *bufp;
    struct re_registers *regs;
    unsigned num_regs;
    regoff_t *starts, *ends;
{
  if (num_regs)
    {
      bufp->regs_allocated = REGS_REALLOCATE;
      regs->num_regs = num_regs;
      regs->start = starts;
      regs->end = ends;
    }
  else
    {
      bufp->regs_allocated = REGS_UNALLOCATED;
      regs->num_regs = 0;
      regs->start = regs->end = (regoff_t *) 0;
    }
}
#ifdef _LIBC
weak_alias (__re_set_registers, re_set_registers)
#endif

/* Entry points compatible with 4.2 BSD regex library.  We don't define
   them unless specifically requested.  */

#if defined _REGEX_RE_COMP || defined _LIBC
int
# ifdef _LIBC
weak_function
# endif
re_exec (s)
     const char *s;
{
  return 0 == regexec (&re_comp_buf, s, 0, NULL, 0);
}
#endif /* _REGEX_RE_COMP */

static re_node_set empty_set;

/* Internal entry point.  */

/* Searches for a compiled pattern PREG in the string STRING, whose
   length is LENGTH.  NMATCH, PMATCH, and EFLAGS have the same
   mingings with regexec.  START, and RANGE have the same meanings
   with re_search.
   Return 0 if we find a match and REG_NOMATCH if not.
   Note: We assume front end functions already check ranges.
   (START + RANGE >= 0 && START + RANGE <= LENGTH)  */

static int
re_search_internal (preg, string, length, start, range, nmatch, pmatch, eflags)
    const regex_t *preg;
    const char *string;
    int length, start, range, eflags;
    size_t nmatch;
    regmatch_t pmatch[];
{
  re_dfa_t *dfa = (re_dfa_t *)preg->buffer;
  re_string_t input;
  re_dfastate_t **state_log;
  int fl_longest_match, match_first, match_last = -1;
  re_match_context_t mctx;
  char *fastmap = ((preg->fastmap != NULL && preg->fastmap_accurate)
                   ? preg->fastmap : NULL);

  /* Check if the DFA haven't been compiled.  */
  if (preg->used == 0 || dfa->init_state == NULL
      || dfa->init_state_word == NULL || dfa->init_state_nl == NULL
      || dfa->init_state_begbuf == NULL)
    return 1;

  re_node_set_init_empty (&empty_set);

  /* We must check the longest matching, if nmatch > 0.  */
  fl_longest_match = (nmatch != 0);

  /* We will log all the DFA states through which the dfa pass,
     if nmatch > 1, or this dfa has "multibyte node", which is a
     back-reference or a node which can accept multibyte character or
     multi character collating element.  */
  if (nmatch > 1 || dfa->has_mb_node)
    state_log = re_malloc (re_dfastate_t *, length + 1);
  else
    state_log = NULL;

  if (preg->syntax & RE_ICASE)
    re_string_construct_toupper (&input, string, length, preg->translate);
  else
    re_string_construct (&input, string, length, preg->translate);

  match_ctx_init (&mctx, eflags, dfa->nbackref * 2);

#ifdef DEBUG
  /* We assume front-end functions already check them.  */
  assert (start + range >= 0 && start + range <= length);
#endif

  /* Check incrementally whether of not the input string match.  */
  for (match_first = start; ;)
    {
      if ((match_first < length
           && (fastmap == NULL
               || fastmap[re_string_byte_at (&input, match_first)]))
          || preg->can_be_null)
        {
#ifdef RE_ENABLE_I18N
          if (MB_CUR_MAX == 1 || re_string_first_byte (&input, match_first))
#endif
            {
              /* We assume that the matching starts from `match_first'.  */
              re_string_set_index (&input, match_first);
              mctx.match_first = mctx.state_log_top = match_first;
              mctx.nbkref_ents = mctx.max_bkref_len = 0;
              match_last = check_matching (preg, &input, &mctx, state_log,
                                           match_first, 0, fl_longest_match);
              if (match_last != -1)
                break;
            }
        }
      /* Update counter.  */
      if (range < 0)
        {
          --match_first;
          if (match_first < start + range)
            break;
        }
      else
        {
          ++match_first;
          if (match_first > start + range)
            break;
        }
    }

  /* Set pmatch[] if we need.  */
  if (match_last != -1 && nmatch > 0)
    {
      int reg_idx;

      /* Initialize registers.  */
      for (reg_idx = 0; reg_idx < nmatch; ++reg_idx)
        pmatch[reg_idx].rm_so = pmatch[reg_idx].rm_eo = -1;

      /* Set the points where matching start/end.  */
      pmatch[0].rm_so = mctx.match_first;
      mctx.match_last = pmatch[0].rm_eo = match_last;

      if (!preg->no_sub && nmatch > 1)
        {
          /* We need the ranges of all the subexpressions.  */
          int halt_node;
          re_dfastate_t *pstate = state_log[match_last];
#ifdef DEBUG
          assert (state_log != NULL);
#endif
          halt_node = check_halt_state_context (preg, pstate, &input,
                                                match_last, eflags);
          sift_states_backward (preg, state_log, &mctx, &input, halt_node);
          set_regs (preg, state_log, &mctx, &input, nmatch, pmatch, halt_node);
        }
    }

  re_free (state_log);
  if (dfa->nbackref)
    match_ctx_free (&mctx);
  re_string_destruct (&input);
  return match_last == -1;
}

/* Acquire an initial state.
   We must select appropriate initial state depending on the context,
   since initial states may have constraints like "\<", "^", etc..  */

static inline re_dfastate_t *
acquire_init_state_context (preg, input, idx, eflags)
    const regex_t *preg;
    const re_string_t *input;
    int idx, eflags;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;

  if (dfa->init_state->has_constraint)
    {
      unsigned int context;
      context =  re_string_context_at (input, idx - 1, eflags,
                                       preg->newline_anchor);
      if (IS_WORD_CONTEXT (context))
        return dfa->init_state_word;
      else if (IS_ORDINARY_CONTEXT (context))
        return dfa->init_state;
      else if (IS_BEGBUF_CONTEXT (context) && IS_NEWLINE_CONTEXT (context))
        return dfa->init_state_begbuf;
      else if (IS_NEWLINE_CONTEXT (context))
        return dfa->init_state_nl;
      else if (IS_BEGBUF_CONTEXT (context))
        /* It is relatively rare case, then calculate on demand.  */
        return re_acquire_state_context (dfa, dfa->init_state->entrance_nodes,
                                         context);
      else
        /* Must not happen?  */
        return dfa->init_state;
    }
  else
    return dfa->init_state;
}

/* Check whether the regular expression match input string INPUT or not,
   and return the index where the matching end, or return -1 if not match.
   FL_SEARCH means we must search where the matching starts,
   FL_LONGEST_MATCH means we want the POSIX longest matching.  */

static int
check_matching (preg, input, mctx, state_log, start_idx, fl_search,
                fl_longest_match)
    const regex_t *preg;
    re_string_t *input;
    re_match_context_t *mctx;
    re_dfastate_t **state_log;
    int start_idx, fl_search, fl_longest_match;
{
  int match = 0, match_last = -1;
  re_dfastate_t *cur_state;

  cur_state = acquire_init_state_context (preg, input, start_idx,
                                          mctx->eflags);
  if (state_log != NULL)
    state_log[start_idx] = cur_state;
  /* If the RE accepts NULL string.  */
  if (cur_state->halt)
    {
      if (!cur_state->has_constraint
          || check_halt_state_context (preg, cur_state, input, start_idx,
                                       mctx->eflags))
        {
          if (!fl_longest_match)
            return start_idx;
          else
            {
              match_last = start_idx;
              match = 1;
            }
        }
    }

  while (!re_string_eoi (input))
    {
      cur_state = transit_state (preg, cur_state, input, fl_search && !match,
                                 state_log, mctx);
      if (cur_state == NULL) /* Reached at the invalid state.  */
        {
          int cur_str_idx = re_string_cur_idx (input);
          if (fl_search && !match)
            {
              /* Restart from initial state, since we are searching
                 the point from where matching start.  */
#ifdef RE_ENABLE_I18N
              if (MB_CUR_MAX == 1 || re_string_first_byte (input, cur_str_idx))
#endif /* RE_ENABLE_I18N */
                cur_state = acquire_init_state_context (preg, input,
                                                        cur_str_idx,
                                                        mctx->eflags);
              if (state_log != NULL)
                state_log[cur_str_idx] = cur_state;
            }
          else if (!fl_longest_match && match)
            break;
          else /* (fl_longest_match && match) || (!fl_search && !match)  */
            {
              if (state_log == NULL)
                break;
              else
                {
                  int max = mctx->state_log_top;
                  for (; cur_str_idx <= max; ++cur_str_idx)
                    if (state_log[cur_str_idx] != NULL)
                      break;
                  if (cur_str_idx > max)
                    break;
                }
            }
        }

      if (cur_state != NULL && cur_state->halt)
        {
          /* Reached at a halt state.
             Check the halt state can satisfy the current context.  */
          if (!cur_state->has_constraint
              || check_halt_state_context (preg, cur_state, input,
                                           re_string_cur_idx (input),
                                           mctx->eflags))
            {
              /* We found an appropriate halt state.  */
              match_last = re_string_cur_idx (input);
              match = 1;
              if (!fl_longest_match)
                break;
            }
        }
   }
  return match_last;
}

/* Check NODE match the current context.  */

static int check_halt_node_context (dfa, node, context)
    const re_dfa_t *dfa;
    int node;
    unsigned int context;
{
  int entity;
  re_token_type_t type = dfa->nodes[node].type;
  if (type == END_OF_RE)
    return 1;
  if (type != OP_CONTEXT_NODE)
    return 0;
  entity = dfa->nodes[node].opr.ctx_info->entity;
  if (dfa->nodes[entity].type != END_OF_RE
      || NOT_SATISFY_NEXT_CONSTRAINT (dfa->nodes[node].constraint, context))
    return 0;
  return 1;
}

/* Check the halt state STATE match the current context.
   Return 0 if not match, if the node, STATE has, is a halt node and
   match the context, return the node.  */

static int
check_halt_state_context (preg, state, input, idx, eflags)
    const regex_t *preg;
    const re_dfastate_t *state;
    const re_string_t *input;
    int idx, eflags;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  int i;
  unsigned int context;
#ifdef DEBUG
  assert (state->halt);
#endif
  context = re_string_context_at (input, idx, eflags, preg->newline_anchor);
  for (i = 0; i < state->nodes.nelem; ++i)
    if (check_halt_node_context (dfa, state->nodes.elems[i], context))
      return state->nodes.elems[i];
  return 0;
}

/* Compute the next node to which "NFA" transit from NODE.
   Return the destination node, and update EPS_VIA_NODES.
   ("NFA" is a NFA corresponding to the DFA.  */

static int
proceed_next_node (preg, state_log, mctx, input, pidx, node, eps_via_nodes)
    const regex_t *preg;
    re_dfastate_t **state_log;
    const re_match_context_t *mctx;
    const re_string_t *input;
    int *pidx, node;
    re_node_set *eps_via_nodes;
{
  re_dfa_t *dfa = (re_dfa_t *)preg->buffer;
  int i, dest_node = -1;
  if (IS_EPSILON_NODE (dfa->nodes[node].type))
    {
      re_node_set_insert (eps_via_nodes, node);
      for (i = 0; i < state_log[*pidx]->nodes.nelem; ++i)
        {
          int candidate = state_log[*pidx]->nodes.elems[i];
          if (!re_node_set_contains (dfa->edests + node, candidate)
              && !(dfa->nodes[candidate].type == OP_CONTEXT_NODE
                   && re_node_set_contains (dfa->edests + node,
                            dfa->nodes[candidate].opr.ctx_info->entity)))
            continue;
          dest_node = candidate;
          /* In order to avoid infinite loop like "(a*)*".  */
          if (!re_node_set_contains (eps_via_nodes, dest_node))
            break;
        }
#ifdef DEBUG
      assert (dest_node != -1);
#endif
      return dest_node;
    }
  else
    {
      int naccepted = 0, entity = node;
      re_token_type_t type = dfa->nodes[node].type;
      if (type == OP_CONTEXT_NODE)
        {
          entity = dfa->nodes[node].opr.ctx_info->entity;
          type = dfa->nodes[entity].type;
        }

      if (ACCEPT_MB_NODE (type))
        naccepted = check_node_accept_bytes (preg, entity, input, *pidx);
      else if (type == OP_BACK_REF)
        {
          for (i = 0; i < mctx->nbkref_ents; ++i)
            {
              if (mctx->bkref_ents[i].node == node
                  && mctx->bkref_ents[i].from == *pidx)
                naccepted = mctx->bkref_ents[i].to - *pidx;
            }
          if (naccepted == 0)
            {
              re_node_set_insert (eps_via_nodes, node);
              dest_node = dfa->nexts[node];
              if (re_node_set_contains (&state_log[*pidx]->nodes, dest_node))
                return dest_node;
              for (i = 0; i < state_log[*pidx]->nodes.nelem; ++i)
                {
                  dest_node = state_log[*pidx]->nodes.elems[i];
                  if ((dfa->nodes[dest_node].type == OP_CONTEXT_NODE
                       && (dfa->nexts[node]
                           == dfa->nodes[dest_node].opr.ctx_info->entity)))
                    return dest_node;
                }
            }
        }

      if (naccepted != 0
          || check_node_accept (preg, dfa->nodes + node, input, *pidx,
                                mctx->eflags))
        {
          dest_node = dfa->nexts[node];
          *pidx = (naccepted == 0) ? *pidx + 1 : *pidx + naccepted;
#ifdef DEBUG
          assert (state_log[*pidx] != NULL);
#endif
          re_node_set_empty (eps_via_nodes);
          return dest_node;
        }
    }
  /* Must not reach here.  */
#ifdef DEBUG
  assert (0);
#endif
  return 0;
}

/* Set the positions where the subexpressions are starts/ends to registers
   PMATCH.
   Note: We assume that pmatch[0] is already set, and
   pmatch[i].rm_so == pmatch[i].rm_eo == -1 (i > 1).  */

static void
set_regs (preg, state_log, mctx, input, nmatch, pmatch, last_node)
    const regex_t *preg;
    re_dfastate_t **state_log;
    const re_match_context_t *mctx;
    const re_string_t *input;
    size_t nmatch;
    regmatch_t *pmatch;
    int last_node;
{
  re_dfa_t *dfa = (re_dfa_t *)preg->buffer;
  int idx, cur_node, node_entity, real_nmatch;
  re_node_set eps_via_nodes;
  int i;
#ifdef DEBUG
  assert (nmatch > 1);
  assert (state_log != NULL);
#endif
  cur_node = dfa->init_node;
  real_nmatch = (nmatch <= preg->re_nsub) ? nmatch : preg->re_nsub + 1;
  re_node_set_init_empty (&eps_via_nodes);
  for (idx = pmatch[0].rm_so; idx <= pmatch[0].rm_eo ;)
    {
      node_entity = ((dfa->nodes[cur_node].type == OP_CONTEXT_NODE)
                     ? dfa->nodes[cur_node].opr.ctx_info->entity : cur_node);
      for (i = 1; i < real_nmatch; ++i)
        {
          if (dfa->subexps[i - 1].start == dfa->subexps[i - 1].end)
            {
              /* In case of the null subexpression like '()'.  */
              if (dfa->subexps[i - 1].start == node_entity)
                {
                  pmatch[i].rm_so = idx;
                  pmatch[i].rm_eo = idx;
                }
            }
          else if (dfa->subexps[i - 1].start <= node_entity
                   && node_entity < dfa->subexps[i - 1].end)
            {
              if (pmatch[i].rm_so == -1 || pmatch[i].rm_eo != -1)
                /* We are at the first node of this sub expression.  */
                {
                  pmatch[i].rm_so = idx;
                  pmatch[i].rm_eo = -1;
                }
            }
          else
            {
              if (pmatch[i].rm_so != -1 && pmatch[i].rm_eo == -1)
                /* We are at the last node of this sub expression.  */
                pmatch[i].rm_eo = idx;
            }
        }
      if (idx == pmatch[0].rm_eo && cur_node == last_node)
        break;

      /* Proceed to next node.  */
      cur_node = proceed_next_node (preg, state_log, mctx, input, &idx,
                                    cur_node, &eps_via_nodes);
    }
  re_node_set_free (&eps_via_nodes);
  return;
}

#define NUMBER_OF_STATE 1

/* This function checks the STATE_LOG from the MCTX->match_last
   to MCTX->match_first and sift the nodes in each states according to
   the following rules.  Updated state_log will be wrote to STATE_LOG.

   Rules: We throw away the Node `a' in the STATE_LOG[STR_IDX] if...
     1. When STR_IDX == MATCH_LAST(the last index in the state_log):
        If `a' isn't the LAST_NODE and `a' can't epsilon transit to
        the LAST_NODE, we throw away the node `a'.
     2. When MATCH_FIRST <= STR_IDX < MATCH_LAST and `a' accepts
        string `s' and transit to `b':
        i. If 'b' isn't in the STATE_LOG[STR_IDX+strlen('s')], we throw
           away the node `a'.
        ii. If 'b' is in the STATE_LOG[STR_IDX+strlen('s')] but 'b' is
            throwed away, we throw away the node `a'.
     3. When 0 <= STR_IDX < n and 'a' epsilon transit to 'b':
        i. If 'b' isn't in the STATE_LOG[STR_IDX], we throw away the
           node `a'.
        ii. If 'b' is in the STATE_LOG[STR_IDX] but 'b' is throwed away,
            we throw away the node `a'.  */

#define STATE_NODE_CONTAINS(state,node) \
  ((state) != NULL && re_node_set_contains (&(state)->nodes, node))

static void
sift_states_backward (preg, state_log, mctx, input, last_node)
    const regex_t *preg;
    re_dfastate_t **state_log;
    const re_match_context_t *mctx;
    const re_string_t *input;
    int last_node;
{
  re_dfa_t *dfa = (re_dfa_t *)preg->buffer;
  re_node_set state_buf;
  int str_idx = mctx->match_last;
  re_node_set *plog;	/* Points the state_log[str_idx]->nodes  */

#ifdef DEBUG
  assert (state_log != NULL && state_log[str_idx] != NULL);
#endif
  re_node_set_alloc (&state_buf, NUMBER_OF_STATE);
  plog = &state_log[str_idx]->nodes;

  /* Build sifted state_log[str_idx].  It has the nodes which can epsilon
     transit to the last_node and the last_node itself.  */
  re_node_set_intersect (&state_buf, plog, dfa->inveclosures + last_node);

  if (state_log[str_idx] != NULL && state_log[str_idx]->has_backref)
    add_epsilon_backreference (dfa, mctx, plog, str_idx, &state_buf);

  /* Update state log.  */
  state_log[str_idx] = re_acquire_state (dfa, &state_buf);

  /* Then check each states in the state_log.  */
  while (str_idx > mctx->match_first)
    {
      int i, j;
      /* Update counters.  */
      re_node_set_empty (&state_buf);
      --str_idx;
      plog = ((state_log[str_idx] == NULL) ? &empty_set
              : &state_log[str_idx]->nodes);

      /* Then build the next sifted state.
         We build the next sifted state on `state_buf', and update
         `state_log[str_idx]' with `state_buf'.
         Note:
         `state_buf' is the sifted state from `state_log[str_idx + 1]'.
         `plog' points the node_set of the old `state_log[str_idx]'.  */
      for (i = 0; i < plog->nelem; i++)
        {
          int prev_node = plog->elems[i];
          int entity = prev_node;
          int naccepted = 0;
          re_token_type_t type = dfa->nodes[prev_node].type;
          if (type == OP_CONTEXT_NODE)
            {
              entity = dfa->nodes[prev_node].opr.ctx_info->entity;
              type = dfa->nodes[entity].type;
            }

          /* If the node may accept `multi byte'.  */
          if (ACCEPT_MB_NODE (type))
            naccepted = sift_states_iter_mb (preg, state_log, mctx, input,
                                             entity, str_idx,
                                             mctx->match_last);

          /* If the node is a back reference.  */
          else if (type == OP_BACK_REF)
            for (j = 0; j < mctx->nbkref_ents; ++j)
              {
                naccepted = sift_states_iter_bkref (dfa, state_log,
                                                    mctx->bkref_ents + j,
                                                    prev_node, str_idx,
                                                    mctx->match_first,
                                                    mctx->match_last);
                if (naccepted)
                  break;
              }

          if (!naccepted
              && check_node_accept (preg, dfa->nodes + prev_node, input,
                                    str_idx, mctx->eflags)
              && STATE_NODE_CONTAINS (state_log[str_idx + 1],
                                      dfa->nexts[prev_node]))
            naccepted = 1;

          if (naccepted == 0)
            continue;

          /* `prev_node' may point the entity of the OP_CONTEXT_NODE,
             then we use plog->elems[i] instead.  */
          re_node_set_add_intersect (&state_buf, plog,
                                     dfa->inveclosures + prev_node);
        }
      if (state_log[str_idx] != NULL && state_log[str_idx]->has_backref)
        add_epsilon_backreference (dfa, mctx, plog, str_idx, &state_buf);

      /* Update state_log.  */
      state_log[str_idx] = re_acquire_state (dfa, &state_buf);
    }

  re_node_set_free (&state_buf);
}

/* Helper functions.  */

static inline void
clean_state_log_if_need (state_log, mctx, next_state_log_idx)
    re_dfastate_t **state_log;
    re_match_context_t *mctx;
    int next_state_log_idx;
{
  int top = mctx->state_log_top;
  if (top < next_state_log_idx)
    {
      memset (state_log + top + 1, '\0',
              sizeof (re_dfastate_t *) * (next_state_log_idx - top));
      mctx->state_log_top = next_state_log_idx;
    }
}

static int
sift_states_iter_mb (preg, state_log, mctx, input, node_idx, str_idx,
                     max_str_idx)
    const regex_t *preg;
    re_dfastate_t **state_log;
    const re_match_context_t *mctx;
    const re_string_t *input;
    int node_idx, str_idx, max_str_idx;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  int naccepted;
  /* Check the node can accept `multi byte'.  */
  naccepted = check_node_accept_bytes (preg, node_idx, input, str_idx);
  if (naccepted > 0 && str_idx + naccepted <= max_str_idx &&
      !STATE_NODE_CONTAINS (state_log[str_idx + naccepted],
                            dfa->nexts[node_idx]))
    /* The node can't accept the `multi byte', or the
       destination was already throwed away, then the node
       could't accept the current input `multi byte'.   */
    naccepted = 0;
  /* Otherwise, it is sure that the node could accept
     `naccepted' bytes input.  */
  return naccepted;
}

static int
sift_states_iter_bkref (dfa, state_log, mctx_entry, node_idx, idx, match_first,
                        match_last)
    const re_dfa_t *dfa;
    re_dfastate_t **state_log;
    struct re_backref_cache_entry *mctx_entry;
    int node_idx, idx, match_first, match_last;
{
  int naccepted = 0;
  int from_idx, to_idx;
  from_idx = mctx_entry->from;
  to_idx = mctx_entry->to;
  if (mctx_entry->node == node_idx
      && from_idx == idx && to_idx <= match_last
      && STATE_NODE_CONTAINS (state_log[to_idx], dfa->nexts[node_idx]))
    naccepted = to_idx - from_idx;
  return naccepted;
}

static void
add_epsilon_backreference (dfa, mctx, plog, idx, state_buf)
    const re_dfa_t *dfa;
    const re_match_context_t *mctx;
    const re_node_set *plog;
    int idx;
    re_node_set *state_buf;
{
  int i, j;
  for (i = 0; i < plog->nelem; ++i)
    {
      int node_idx = plog->elems[i];
      re_token_type_t type = dfa->nodes[node_idx].type;
      if (type == OP_CONTEXT_NODE)
        type = dfa->nodes[dfa->nodes[node_idx].opr.ctx_info->entity].type;

      if (type == OP_BACK_REF &&
          !re_node_set_contains (state_buf, node_idx))
        {
          for (j = 0; j < mctx->nbkref_ents; ++j)
            {
              struct re_backref_cache_entry *entry;
              entry = mctx->bkref_ents + j;
              if (entry->from == entry->to && entry->from == idx)
                break;
            }
          if (j < mctx->nbkref_ents || idx == mctx->match_first)
            {
              re_node_set_add_intersect (state_buf, plog,
                                         dfa->inveclosures + node_idx);
              i = 0;
            }
        }
    }
}

/* Functions for state transition.  */

/* Return the next state to which the current state STATE will transit by
   accepting the current input byte, and update STATE_LOG if necessary.
   If STATE can accept a multibyte char/collating element/back reference
   update the destination of STATE_LOG.  */

static re_dfastate_t *
transit_state (preg, state, input, fl_search, state_log, mctx)
    const regex_t *preg;
    re_dfastate_t *state, **state_log;
    re_string_t *input;
    int fl_search;
    re_match_context_t *mctx;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  re_dfastate_t **trtable, *next_state;
  unsigned char ch;

  if (state == NULL)
    {
      next_state = state;
      re_string_skip_bytes (input, 1);
    }
  else
    {
      /* If the current state can accept multibyte.  */
      if (state->accept_mb)
        transit_state_mb (preg, state, input, state_log, mctx);

      /* Then decide the next state with the single byte.  */
      if (1)
        {
          /* Use transition table  */
          ch = re_string_fetch_byte (input);
          trtable = fl_search ? state->trtable_search : state->trtable;
          if (trtable == NULL)
            {
              trtable = build_trtable (preg, state, fl_search);
              if (fl_search)
                state->trtable_search = trtable;
              else
                state->trtable = trtable;
            }
          next_state = trtable[ch];
        }
      else
        {
          /* don't use transition table  */
          next_state = transit_state_sb (preg, state, input, fl_search, mctx);
        }
    }

  /* Update the state_log if we need.  */
  if (state_log != NULL)
    {
      int cur_idx = re_string_cur_idx (input);
      if (cur_idx > mctx->state_log_top)
        {
          state_log[cur_idx] = next_state;
          mctx->state_log_top = cur_idx;
        }
      else if (state_log[cur_idx] == 0)
        {
          state_log[cur_idx] = next_state;
        }
      else
        {
          re_dfastate_t *pstate;
          unsigned int context;
          re_node_set next_nodes, *log_nodes, *table_nodes = NULL;
          /* If (state_log[cur_idx] != 0), it implies that cur_idx is
             the destination of a multibyte char/collating element/
             back reference.  Then the next state is the union set of
             these destinations and the results of the transition table.  */
          pstate = state_log[cur_idx];
          log_nodes = pstate->entrance_nodes;
          if (next_state != NULL)
            {
              table_nodes = next_state->entrance_nodes;
              re_node_set_init_union (&next_nodes, table_nodes, log_nodes);
            }
          else
            next_nodes = *log_nodes;
          /* Note: We already add the nodes of the initial state,
                   then we don't need to add them here.  */

          context = re_string_context_at (input, re_string_cur_idx (input) - 1,
                                          mctx->eflags, preg->newline_anchor);
          next_state = state_log[cur_idx]
              = re_acquire_state_context (dfa, &next_nodes, context);
          if (table_nodes != NULL)
            re_node_set_free (&next_nodes);
        }
      /* If the next state has back references.  */
      if (next_state != NULL && next_state->has_backref)
        {
          transit_state_bkref (preg, next_state, input, state_log, mctx);
          next_state = state_log[cur_idx];
        }
    }
  return next_state;
}

/* Helper functions for transit_state.  */

/* Return the next state to which the current state STATE will transit by
   accepting the current input byte.  */

static re_dfastate_t *
transit_state_sb (preg, state, input, fl_search, mctx)
    const regex_t *preg;
    re_dfastate_t *state;
    re_string_t *input;
    int fl_search;
    re_match_context_t *mctx;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  re_node_set next_nodes;
  re_dfastate_t *next_state;
  int node_cnt, cur_str_idx = re_string_cur_idx (input);
  unsigned int context;

  re_node_set_alloc (&next_nodes, state->nodes.nelem + 1);
  for (node_cnt = 0; node_cnt < state->nodes.nelem; ++node_cnt)
    {
      int cur_node = state->nodes.elems[node_cnt];
      if (check_node_accept (preg, dfa->nodes + cur_node, input,
                             cur_str_idx, mctx->eflags))
        re_node_set_merge (&next_nodes,
                           dfa->eclosures + dfa->nexts[cur_node]);
    }
  if (fl_search)
    {
#ifdef RE_ENABLE_I18N
      int not_initial = 0;
      if (MB_CUR_MAX > 1)
        for (node_cnt = 0; node_cnt < next_nodes.nelem; ++node_cnt)
          if (dfa->nodes[next_nodes.elems[node_cnt]].type == CHARACTER)
            {
              not_initial = dfa->nodes[next_nodes.elems[node_cnt]].mb_partial;
              break;
            }
      if (!not_initial)
#endif
        re_node_set_merge (&next_nodes, dfa->init_state->entrance_nodes);
    }
  context = re_string_context_at (input, cur_str_idx, mctx->eflags,
                                  preg->newline_anchor);
  next_state = re_acquire_state_context (dfa, &next_nodes, context);
  re_node_set_free (&next_nodes);
  re_string_skip_bytes (input, 1);
  return next_state;
}

static void
transit_state_mb (preg, pstate, input, state_log, mctx)
    const regex_t *preg;
    re_dfastate_t *pstate, **state_log;
    const re_string_t *input;
    re_match_context_t *mctx;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  int i;

  for (i = 0; i < pstate->nodes.nelem; ++i)
    {
      re_node_set dest_nodes, *new_nodes;
      int cur_node_idx = pstate->nodes.elems[i];
      int naccepted = 0, dest_idx;
      unsigned int context;
      re_dfastate_t *dest_state;

      if (dfa->nodes[cur_node_idx].type == OP_CONTEXT_NODE)
        {
          context = re_string_context_at (input, re_string_cur_idx (input),
                                          mctx->eflags, preg->newline_anchor);
          if (NOT_SATISFY_NEXT_CONSTRAINT (dfa->nodes[cur_node_idx].constraint,
                                        context))
            continue;
          cur_node_idx = dfa->nodes[cur_node_idx].opr.ctx_info->entity;
        }

      /* How many bytes the node can accepts?  */
      if (ACCEPT_MB_NODE (dfa->nodes[cur_node_idx].type))
        naccepted = check_node_accept_bytes (preg, cur_node_idx, input,
                                             re_string_cur_idx (input));
      if (naccepted == 0)
        continue;

      /* The node can accepts `naccepted' bytes.  */
      dest_idx = re_string_cur_idx (input) + naccepted;
      clean_state_log_if_need (state_log, mctx, dest_idx);
#ifdef DEBUG
      assert (dfa->nexts[cur_node_idx] != -1);
#endif
      /* `cur_node_idx' may point the entity of the OP_CONTEXT_NODE,
         then we use pstate->nodes.elems[i] instead.  */
      new_nodes = dfa->eclosures + dfa->nexts[pstate->nodes.elems[i]];

      dest_state = state_log[dest_idx];
      if (dest_state == NULL)
        dest_nodes = *new_nodes;
      else
        re_node_set_init_union (&dest_nodes, dest_state->entrance_nodes,
                                new_nodes);
      context = re_string_context_at (input, dest_idx - 1, mctx->eflags,
                                      preg->newline_anchor);
      state_log[dest_idx] = re_acquire_state_context (dfa, &dest_nodes, context);
      if (dest_state != NULL)
        re_node_set_free (&dest_nodes);
    }
}

static void
transit_state_bkref (preg, pstate, input, state_log, mctx)
    const regex_t *preg;
    re_dfastate_t *pstate, **state_log;
    const re_string_t *input;
    re_match_context_t *mctx;
{
  re_dfastate_t **work_state_log;

#ifdef DEBUG
  assert (mctx->match_first != -1);
#endif
  work_state_log = re_malloc (re_dfastate_t *, re_string_cur_idx (input) + 1);

  transit_state_bkref_loop (preg, input, &pstate->nodes, work_state_log,
                            state_log, mctx);

  re_free (work_state_log);
}

/* Caller must allocate `work_state_log'.  */

static void
transit_state_bkref_loop (preg, input, nodes, work_state_log, state_log, mctx)
    const regex_t *preg;
    const re_string_t *input;
    re_node_set *nodes;
    re_dfastate_t **work_state_log, **state_log;
    re_match_context_t *mctx;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  int i, j;
  regmatch_t *cur_regs = re_malloc (regmatch_t, preg->re_nsub + 1);
  int cur_str_idx = re_string_cur_idx (input);

  for (i = 0; i < nodes->nelem; ++i)
    {
      int dest_str_idx, subexp_idx, prev_nelem, subexp_len;
      int node_idx = nodes->elems[i];
      unsigned int context;
      re_token_t *node = dfa->nodes + node_idx;
      re_dfastate_t *dest_state;
      re_node_set *new_dest_nodes;

      /* Check whether `node' is a backreference or not.  */
      if (node->type == OP_BACK_REF)
        subexp_idx = node->opr.idx;
      else if (node->type == OP_CONTEXT_NODE &&
               dfa->nodes[node->opr.ctx_info->entity].type == OP_BACK_REF)
        {
          context = re_string_context_at (input, cur_str_idx, mctx->eflags,
                                          preg->newline_anchor);
          if (NOT_SATISFY_NEXT_CONSTRAINT (node->constraint, context))
            continue;
          subexp_idx = dfa->nodes[node->opr.ctx_info->entity].opr.idx;
        }
      else
        continue;

      /* `node' is a backreference.
         At first, set registers to check the backreference. */
      cur_regs[0].rm_so = mctx->match_first;
      cur_regs[0].rm_eo = cur_str_idx;
      memcpy (work_state_log + mctx->match_first,
              state_log + mctx->match_first,
              sizeof (re_dfastate_t *)
	      * (cur_str_idx - mctx->match_first + 1));
      mctx->match_last = cur_str_idx;
      sift_states_backward (preg, work_state_log, mctx, input, node_idx);
      if (!STATE_NODE_CONTAINS (work_state_log[mctx->match_first],
                                dfa->init_node))
        continue;
      for (j = 1; j <= preg->re_nsub; ++j)
        cur_regs[j].rm_so = cur_regs[j].rm_eo = -1;
      set_regs (preg, work_state_log, mctx, input,
                subexp_idx + 1, cur_regs, node_idx);

      /* Then check that the backreference can match the input string.  */
      subexp_len = cur_regs[subexp_idx].rm_eo - cur_regs[subexp_idx].rm_so;
      if (subexp_len < 0
          || (strncmp ((re_string_get_buffer (input)
                        + cur_regs[subexp_idx].rm_so),
                       re_string_get_buffer (input) + cur_str_idx, subexp_len)
              != 0))
        continue;

      /* Successfully matched, add a new cache entry.  */
      dest_str_idx = cur_str_idx + subexp_len;
      match_ctx_add_entry (mctx, node_idx, cur_str_idx, dest_str_idx);
      clean_state_log_if_need (state_log, mctx, dest_str_idx);

      /* And add the epsilon closures (which is `new_dest_nodes') of
         the backreference to appropriate state_log.  */
#ifdef DEBUG
      assert (dfa->nexts[node_idx] != -1);
#endif
      if (node->type == OP_CONTEXT_NODE && subexp_len == 0)
        new_dest_nodes = dfa->nodes[node_idx].opr.ctx_info->bkref_eclosure;
      else
        new_dest_nodes = dfa->eclosures + dfa->nexts[node_idx];
      context = (IS_WORD_CHAR (re_string_byte_at (input, dest_str_idx - 1))
                 ? CONTEXT_WORD : 0);
      dest_state = state_log[dest_str_idx];

      prev_nelem = ((state_log[cur_str_idx] == NULL) ? 0
                    : state_log[cur_str_idx]->nodes.nelem);
      /* Add `new_dest_node' to state_log.  */
      if (dest_state == NULL)
        state_log[dest_str_idx] = re_acquire_state_context (dfa,
                                                            new_dest_nodes,
                                                            context);
      else
        {
          re_node_set dest_nodes;
          re_node_set_init_union (&dest_nodes, dest_state->entrance_nodes,
                                  new_dest_nodes);
          state_log[dest_str_idx] = re_acquire_state_context (dfa, &dest_nodes,
                                                              context);
          re_node_set_free (&dest_nodes);
        }

      /* We need to check recursively if the backreference can epsilon
         transit.  */
      if (subexp_len == 0 && state_log[cur_str_idx]->nodes.nelem > prev_nelem)
        transit_state_bkref_loop (preg, input, new_dest_nodes, work_state_log,
                                  state_log, mctx);
    }
  re_free (cur_regs);
}

/* Build transition table for the state.  */

static re_dfastate_t **
build_trtable (preg, state, fl_search)
    const regex_t *preg;
    const re_dfastate_t *state;
    int fl_search;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  int i, j, k, ch;
  int ndests; /* Number of the destination states from `state'.  */
  re_dfastate_t **trtable, **dest_states, **dest_states_word, **dest_states_nl;
  re_node_set follows, *dests_node;
  bitset *dests_ch;
  bitset acceptable;

  /* We build DFA states which corresponds to the destination nodes
     from `state'.  `dests_node[i]' represents the nodes which i-th
     destination state contains, and `dests_ch[i]' represents the
     characters which i-th destination state accepts.  */
  dests_node = re_malloc (re_node_set, SBC_MAX);
  dests_ch = re_malloc (bitset, SBC_MAX);

  /* Initialize transiton table.  */
  trtable = (re_dfastate_t **) calloc (sizeof (re_dfastate_t *), SBC_MAX);

  /* At first, group all nodes belonging to `state' into several
     destinations.  */
  ndests = group_nodes_into_DFAstates (preg, state, dests_node, dests_ch);
  if (ndests == 0)
    {
      re_free (dests_node);
      re_free (dests_ch);
      return trtable;
    }

  dest_states = re_malloc (re_dfastate_t *, ndests);
  dest_states_word = re_malloc (re_dfastate_t *, ndests);
  dest_states_nl = re_malloc (re_dfastate_t *, ndests);
  bitset_empty (acceptable);

  re_node_set_alloc (&follows, ndests + 1);
  /* Then build the states for all destinations.  */
  for (i = 0; i < ndests; ++i)
    {
      int next_node;
      re_node_set_empty (&follows);
      /* Merge the follows of this destination states.  */
      for (j = 0; j < dests_node[i].nelem; ++j)
        {
          next_node = dfa->nexts[dests_node[i].elems[j]];
          if (next_node != -1)
            {
              re_node_set_merge (&follows, dfa->eclosures + next_node);
            }
        }
      /* If search flag is set, merge the initial state.  */
      if (fl_search)
        {
#ifdef RE_ENABLE_I18N
          int not_initial = 0;
          for (j = 0; j < follows.nelem; ++j)
            if (dfa->nodes[follows.elems[j]].type == CHARACTER)
              {
                not_initial = dfa->nodes[follows.elems[j]].mb_partial;
                break;
              }
          if (!not_initial)
#endif
            re_node_set_merge (&follows, dfa->init_state->entrance_nodes);
        }
      dest_states[i] = re_acquire_state_context (dfa, &follows, 0);
      /* If the new state has context constraint,
         build appropriate states for these contexts.  */
      if (dest_states[i]->has_constraint)
        {
          dest_states_word[i] = re_acquire_state_context (dfa, &follows,
                                                          CONTEXT_WORD);
          dest_states_nl[i] = re_acquire_state_context (dfa, &follows,
                                                        CONTEXT_NEWLINE);
        }
      else
        {
          dest_states_word[i] = dest_states[i];
          dest_states_nl[i] = dest_states[i];
        }
      bitset_merge (acceptable, dests_ch[i]);
    }

  /* Update the transition table.  */
  for (i = 0, ch = 0; i < BITSET_UINTS; ++i)
    for (j = 0; j < UINT_BITS; ++j, ++ch)
      if ((acceptable[i] >> j) & 1)
        {
          if (IS_WORD_CHAR (ch))
            {
              for (k = 0; k < ndests; ++k)
                if ((dests_ch[k][i] >> j) & 1)
                  trtable[ch] = dest_states_word[k];
            }
          else /* not WORD_CHAR */
            {
              for (k = 0; k < ndests; ++k)
                if ((dests_ch[k][i] >> j) & 1)
                  trtable[ch] = dest_states[k];
            }
        }
  /* new line */
  for (k = 0; k < ndests; ++k)
    if (bitset_contain (acceptable, NEWLINE_CHAR))
      trtable[NEWLINE_CHAR] = dest_states_nl[k];

  re_free (dest_states_nl);
  re_free (dest_states_word);
  re_free (dest_states);

  re_node_set_free (&follows);
  for (i = 0; i < ndests; ++i)
    re_node_set_free (dests_node + i);

  re_free (dests_ch);
  re_free (dests_node);

  return trtable;
}

/* Group all nodes belonging to STATE into several destinations.
   Then for all destinations, set the nodes belonging to the destination
   to DESTS_NODE[i] and set the characters accepted by the destination
   to DEST_CH[i].  This function return the number of destinations.  */

static int
group_nodes_into_DFAstates (preg, state, dests_node, dests_ch)
    const regex_t *preg;
    const re_dfastate_t *state;
    re_node_set *dests_node;
    bitset *dests_ch;
{
  const re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  int i, j, k;
  int ndests; /* Number of the destinations from `state'.  */
  bitset accepts; /* Characters a node can accept.  */
  const re_node_set *cur_nodes = &state->nodes;
  bitset_empty (accepts);
  ndests = 0;

  /* For all the nodes belonging to `state',  */
  for (i = 0; i < cur_nodes->nelem; ++i)
    {
      unsigned int constraint = 0;
      re_token_t *node = &dfa->nodes[cur_nodes->elems[i]];
      re_token_type_t type = node->type;

      if (type == OP_CONTEXT_NODE)
        {
          constraint = node->constraint;
          node = dfa->nodes + node->opr.ctx_info->entity;
          type = node->type;
        }

      /* Enumerate all single byte character this node can accept.  */
      if (type == CHARACTER)
        bitset_set (accepts, node->opr.c);
      else if (type == SIMPLE_BRACKET)
        {
          bitset_merge (accepts, node->opr.sbcset);
        }
      else if (type == OP_PERIOD)
        {
          bitset_set_all (accepts);
          if (!(preg->syntax & RE_DOT_NEWLINE))
            bitset_clear (accepts, '\n');
          if (preg->syntax & RE_DOT_NOT_NULL)
            bitset_clear (accepts, '\0');
        }
      else
        continue;

      /* Check the `accepts' and sift the characters which are not
         match it the context.  */
      if (constraint)
        {
          if (constraint & NEXT_WORD_CONSTRAINT)
            for (j = 0; j < BITSET_UINTS; ++j)
              accepts[j] &= dfa->word_char[j];
          else if (constraint & NEXT_NOTWORD_CONSTRAINT)
            for (j = 0; j < BITSET_UINTS; ++j)
              accepts[j] &= ~dfa->word_char[j];
          else if (constraint & NEXT_NEWLINE_CONSTRAINT)
            {
              int accepts_newline = bitset_contain (accepts, NEWLINE_CHAR);
              bitset_empty (accepts);
              if (accepts_newline)
                bitset_set (accepts, NEWLINE_CHAR);
              else
                continue;
            }
        }

      /* Then divide `accepts' into DFA states, or create a new
         state.  */
      for (j = 0; j < ndests; ++j)
        {
          bitset intersec; /* Intersection sets, see below.  */
          bitset remains;
          /* Flags, see below.  */
          int has_intersec, not_subset, not_consumed;

          /* Optimization, skip if this state doesn't accept the character.  */
          if (type == CHARACTER && !bitset_contain (dests_ch[j], node->opr.c))
            continue;

          /* Enumerate the intersection set of this state and `accepts'.  */
          has_intersec = 0;
          for (k = 0; k < BITSET_UINTS; ++k)
            has_intersec |= intersec[k] = accepts[k] & dests_ch[j][k];
          /* And skip if the intersection set is empty.  */
          if (!has_intersec)
            continue;

          /* Then check if this state is a subset of `accepts'.  */
          not_subset = not_consumed = 0;
          for (k = 0; k < BITSET_UINTS; ++k)
            {
              not_subset |= remains[k] = ~accepts[k] & dests_ch[j][k];
              not_consumed |= accepts[k] = accepts[k] & ~dests_ch[j][k];
            }

          /* If this state isn't a subset of `accepts', create a
             new group state, which has the `remains'. */
          if (not_subset)
            {
              bitset_copy (dests_ch[ndests], remains);
              bitset_copy (dests_ch[j], intersec);
              re_node_set_init_copy (dests_node + ndests, &dests_node[j]);
              ++ndests;
            }

          /* Put the position in the current group. */
          re_node_set_insert (&dests_node[j], cur_nodes->elems[i]);

          /* If all characters are consumed, go to next node. */
          if (!not_consumed)
            break;
        }
      /* Some characters remain, create a new group. */
      if (j == ndests)
        {
          bitset_copy (dests_ch[ndests], accepts);
          re_node_set_init_1 (dests_node + ndests, cur_nodes->elems[i]);
          ++ndests;
          bitset_empty (accepts);
        }
    }
  return ndests;
}

/* Check how many bytes the node `dfa->nodes[node_idx]' accepts.  */

static int
check_node_accept_bytes (preg, node_idx, input, str_idx)
    const regex_t *preg;
    int node_idx, str_idx;
    const re_string_t *input;
{
  const re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  const re_token_t *node = dfa->nodes + node_idx;
  int elem_len = re_string_elem_size_at (input, str_idx);
  int char_len = re_string_char_size_at (input, str_idx);
  int i, j;
#ifdef _LIBC
  uint32_t nrules = _NL_CURRENT_WORD (LC_COLLATE, _NL_COLLATE_NRULES);
#endif /* _LIBC */
  if (elem_len <= 1 && char_len <= 1)
    return 0;
  if (node->type == OP_PERIOD)
    {
      if ((!(preg->syntax & RE_DOT_NEWLINE) &&
           re_string_byte_at (input, str_idx) == '\n') ||
          ((preg->syntax & RE_DOT_NOT_NULL) &&
           re_string_byte_at (input, str_idx) == '\0'))
        return 0;
      return char_len;
    }
  else if (node->type == COMPLEX_BRACKET)
    {
      const re_charset_t *cset = node->opr.mbcset;
      const unsigned char *pin = re_string_get_buffer (input) + str_idx;
#ifdef _LIBC
      if (nrules != 0)
        {
          int match_len = 0;
          unsigned int in_collseq = 0;
          const int32_t *table, *indirect;
          const unsigned char *weights, *extra, *collseqwc;
          int32_t idx;
          wchar_t wc = 0;
          /* This #include defines a local function!  */
# include <locale/weight.h>

          /* match with collating_symbol?  */
          if (cset->ncoll_syms)
            extra = (const unsigned char *)
              _NL_CURRENT (LC_COLLATE, _NL_COLLATE_SYMB_EXTRAMB);
          for (i = 0; i < cset->ncoll_syms; ++i)
            {
              const unsigned char *coll_sym = extra + cset->coll_syms[i];
              /* Compare the length of input collating element and
                 the length of current collating element.  */
              if (*coll_sym != elem_len)
                continue;
              /* Compare each bytes.  */
              for (j = 0; j < *coll_sym; j++)
                if (pin[j] != coll_sym[1 + j])
                  break;
              if (j == *coll_sym)
                {
                  /* Match if every bytes is equal.  */
                  match_len = j;
                  goto check_node_accept_bytes_match;
                }
            }

          if (cset->nranges || cset->nchar_classes || cset->nmbchars)
            wc = re_string_wchar_at (input, str_idx);

          if (cset->nranges)
            {
              if (elem_len <= char_len)
                {
                  collseqwc = _NL_CURRENT (LC_COLLATE, _NL_COLLATE_COLLSEQWC);
                  in_collseq = collseq_table_lookup (collseqwc, wc);
                }
              else
                in_collseq = find_collation_sequence_value (pin, elem_len);
            }
          /* match with range expression?  */
          for (i = 0; i < cset->nranges; ++i)
            if (cset->range_starts[i] <= in_collseq
                && in_collseq <= cset->range_ends[i])
              {
                match_len = elem_len;
                goto check_node_accept_bytes_match;
              }

          /* match with equivalence_class?  */
          if (cset->nequiv_classes)
            {
              const unsigned char *cp = pin;
              table = (const int32_t *)
                _NL_CURRENT (LC_COLLATE, _NL_COLLATE_TABLEMB);
              weights = (const unsigned char *)
                _NL_CURRENT (LC_COLLATE, _NL_COLLATE_WEIGHTMB);
              extra = (const unsigned char *)
                _NL_CURRENT (LC_COLLATE, _NL_COLLATE_EXTRAMB);
              indirect = (const int32_t *)
                _NL_CURRENT (LC_COLLATE, _NL_COLLATE_INDIRECTMB);
              idx = findidx (&cp);
              if (idx > 0)
                for (i = 0; i < cset->nequiv_classes; ++i)
                  {
                    int32_t equiv_class_idx = cset->equiv_classes[i];
                    size_t weight_len = weights[idx];
                    if (weight_len == weights[equiv_class_idx])
                      {
                        int cnt = 0;
                        while (cnt <= weight_len
                               && (weights[equiv_class_idx + 1 + cnt]
                                   == weights[idx + 1 + cnt]))
                          ++cnt;
                        if (cnt > weight_len)
                          {
                            match_len = elem_len;
                            goto check_node_accept_bytes_match;
                          }
                      }
                  }
            }

          /* match with multibyte character?  */
          for (i = 0; i < cset->nmbchars; ++i)
            if (wc == cset->mbchars[i])
              {
                match_len = char_len;
                goto check_node_accept_bytes_match;
              }

          /* match with character_class?  */
          for (i = 0; i < cset->nchar_classes; ++i)
            {
              wctype_t wt = cset->char_classes[i];
              if (__iswctype (wc, wt))
                {
                  match_len = char_len;
                  goto check_node_accept_bytes_match;
                }
            }

        check_node_accept_bytes_match:
          if (!cset->non_match)
            return match_len;
          else
            {
              if (match_len > 0)
                return 0;
              else
                return re_string_elem_size_at (input, str_idx);
            }
        }
#endif
    }
  return 0;
}

#ifdef _LIBC
static unsigned int
find_collation_sequence_value (mbs, mbs_len)
    const unsigned char *mbs;
    size_t mbs_len;
{
  uint32_t nrules = _NL_CURRENT_WORD (LC_COLLATE, _NL_COLLATE_NRULES);
  if (nrules == 0)
    {
      if (mbs_len == 1)
        {
          /* No valid character.  Match it as a single byte character.  */
          const unsigned char *collseq = (const unsigned char *)
            _NL_CURRENT (LC_COLLATE, _NL_COLLATE_COLLSEQMB);
          return collseq[mbs[0]];
        }
      return UINT_MAX;
    }
  else
    {
      int32_t idx;
      const unsigned char *extra = (const unsigned char *)
        _NL_CURRENT (LC_COLLATE, _NL_COLLATE_SYMB_EXTRAMB);

      for (idx = 0; ;)
        {
          int mbs_cnt, found = 0;
          int32_t elem_mbs_len;
          /* Skip the name of collating element name.  */
          idx = idx + extra[idx] + 1;
          elem_mbs_len = extra[idx++];
          if (mbs_len == elem_mbs_len)
            {
              for (mbs_cnt = 0; mbs_cnt < elem_mbs_len; ++mbs_cnt)
                if (extra[idx + mbs_cnt] != mbs[mbs_cnt])
                  break;
              if (mbs_cnt == elem_mbs_len)
                /* Found the entry.  */
                found = 1;
            }
          /* Skip the byte sequence of the collating element.  */
          idx += elem_mbs_len;
          /* Adjust for the alignment.  */
          idx = (idx + 3) & ~3;
          /* Skip the collation sequence value.  */
          idx += sizeof (uint32_t);
          /* Skip the wide char sequence of the collating element.  */
          idx = idx + sizeof (uint32_t) * (extra[idx] + 1);
          /* If we found the entry, return the sequence value.  */
          if (found)
            return *(uint32_t *) (extra + idx);
          /* Skip the collation sequence value.  */
          idx += sizeof (uint32_t);
        }
    }
}
#endif

/* Check whether the node accepts the byte which is IDX-th
   byte of the INPUT.  */

static int
check_node_accept (preg, node, input, idx, eflags)
    const regex_t *preg;
    const re_token_t *node;
    const re_string_t *input;
    int idx, eflags;
{
  const re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  const re_token_t *cur_node;
  unsigned char ch;
  if (node->type == OP_CONTEXT_NODE)
    {
      /* The node has constraints.  Check whether the current context
         satisfies the constraints.  */
      unsigned int context = re_string_context_at (input, idx, eflags,
                                                   preg->newline_anchor);
      if (NOT_SATISFY_NEXT_CONSTRAINT (node->constraint, context))
        return 0;
      cur_node = dfa->nodes + node->opr.ctx_info->entity;
    }
  else
    cur_node = node;

  ch = re_string_byte_at (input, idx);
  if (cur_node->type == CHARACTER)
    return cur_node->opr.c == ch;
  else if (cur_node->type == SIMPLE_BRACKET)
    return bitset_contain (cur_node->opr.sbcset, ch);
  else if (cur_node->type == OP_PERIOD)
    return !((ch == '\n' && !(preg->syntax & RE_DOT_NEWLINE))
             || (ch == '\0' && (preg->syntax & RE_DOT_NOT_NULL)));
  else
    return 0;
}

/* Functions for matching context.  */

static void
match_ctx_init (mctx, eflags, n)
    re_match_context_t *mctx;
    int eflags;
    int n;
{
  mctx->eflags = eflags;
  mctx->match_first = mctx->match_last = -1;
  if (n > 0)
    mctx->bkref_ents = re_malloc (struct re_backref_cache_entry, n);
  else
    mctx->bkref_ents = NULL;
  mctx->nbkref_ents = 0;
  mctx->abkref_ents = n;
  mctx->max_bkref_len = 0;
}

static void
match_ctx_free (mctx)
    re_match_context_t *mctx;
{
  re_free (mctx->bkref_ents);
}

/* Add a new backreference entry to the cache.  */

static void
match_ctx_add_entry (mctx, node, from, to)
    re_match_context_t *mctx;
    int node, from, to;
{
  if (mctx->nbkref_ents >= mctx->abkref_ents)
    {
      mctx->bkref_ents = re_realloc (mctx->bkref_ents,
                                     struct re_backref_cache_entry,
                                     mctx->abkref_ents * 2);
      memset (mctx->bkref_ents + mctx->nbkref_ents, '\0',
             sizeof (struct re_backref_cache_entry) * mctx->abkref_ents);
      mctx->abkref_ents *= 2;
    }
  mctx->bkref_ents[mctx->nbkref_ents].node = node;
  mctx->bkref_ents[mctx->nbkref_ents].from = from;
  mctx->bkref_ents[mctx->nbkref_ents++].to = to;
  if (mctx->max_bkref_len < to - from)
    mctx->max_bkref_len = to - from;
}
