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

static reg_errcode_t match_ctx_init (re_match_context_t *cache, int eflags,
				     re_string_t *input, int n);
static void match_ctx_free (re_match_context_t *cache);
static reg_errcode_t match_ctx_add_entry (re_match_context_t *cache, int node,
                                          int from, int to);
static reg_errcode_t re_search_internal (const regex_t *preg,
                                         const char *string, int length,
                                         int start, int range, int stop,
                                         size_t nmatch, regmatch_t pmatch[],
                                         int eflags);
static int re_search_2_stub (struct re_pattern_buffer *bufp,
                             const char *string1, int length1,
                             const char *string2, int length2,
                             int start, int range, struct re_registers *regs,
                             int stop, int ret_len);
static int re_search_stub (struct re_pattern_buffer *bufp,
                           const char *string, int length, int start,
                           int range, int stop, struct re_registers *regs,
                           int ret_len);
static unsigned re_copy_regs (struct re_registers *regs, regmatch_t *pmatch,
                              int nregs, int regs_allocated);
static inline re_dfastate_t *acquire_init_state_context (reg_errcode_t *err,
                                                         const regex_t *preg,
                                                         const re_match_context_t *mctx,
                                                         int idx);
static int check_matching (const regex_t *preg, re_match_context_t *mctx,
                           int fl_search, int fl_longest_match);
static int check_halt_node_context (const re_dfa_t *dfa, int node,
                                    unsigned int context);
static int check_halt_state_context (const regex_t *preg,
                                     const re_dfastate_t *state,
                                     const re_match_context_t *mctx, int idx);
static void update_regs (re_dfa_t *dfa, regmatch_t *pmatch, int cur_node,
                         int cur_idx, int nmatch);
static int proceed_next_node (const regex_t *preg,
                              const re_match_context_t *mctx,
                              int *pidx, int node, re_node_set *eps_via_nodes);
static reg_errcode_t set_regs (const regex_t *preg,
                               const re_match_context_t *mctx,
                               size_t nmatch, regmatch_t *pmatch, int last);
#ifdef RE_ENABLE_I18N
static int sift_states_iter_mb (const regex_t *preg,
                                const re_match_context_t *mctx,
                                int node_idx, int str_idx, int max_str_idx);
#endif /* RE_ENABLE_I18N */
static int sift_states_iter_bkref (const re_dfa_t *dfa,
                                   re_dfastate_t **state_log,
                                   struct re_backref_cache_entry *mctx_entry,
                                   int node_idx, int idx, int match_last);
static reg_errcode_t sift_states_backward (const regex_t *preg,
                                           const re_match_context_t *mctx,
                                           int last_node);
static reg_errcode_t clean_state_log_if_need (re_match_context_t *mctx,
                                              int next_state_log_idx);
static reg_errcode_t add_epsilon_backreference (const re_dfa_t *dfa,
                                                const re_match_context_t *mctx,
                                                const re_node_set *plog,
                                                int idx,
                                                re_node_set *state_buf);
static re_dfastate_t *transit_state (reg_errcode_t *err, const regex_t *preg,
                                     re_match_context_t *mctx,
                                     re_dfastate_t *state, int fl_search);
static re_dfastate_t *transit_state_sb (reg_errcode_t *err, const regex_t *preg,
                                        re_dfastate_t *pstate,
                                        int fl_search,
                                        re_match_context_t *mctx);
#ifdef RE_ENABLE_I18N
static reg_errcode_t transit_state_mb (const regex_t *preg,
                                       re_dfastate_t *pstate,
                                       re_match_context_t *mctx);
#endif /* RE_ENABLE_I18N */
static reg_errcode_t transit_state_bkref (const regex_t *preg,
                                          re_dfastate_t *pstate,
                                          re_match_context_t *mctx);
static reg_errcode_t transit_state_bkref_loop (const regex_t *preg,
                                               re_node_set *nodes,
                                               re_dfastate_t **work_state_log,
                                               re_match_context_t *mctx);
static re_dfastate_t **build_trtable (const regex_t *dfa,
                                      const re_dfastate_t *state,
                                      int fl_search);
#ifdef RE_ENABLE_I18N
static int check_node_accept_bytes (const regex_t *preg, int node_idx,
                                    const re_string_t *input, int idx);
# ifdef _LIBC
static unsigned int find_collation_sequence_value (const char *mbs,
                                                   size_t name_len);
# endif /* _LIBC */
#endif /* RE_ENABLE_I18N */
static int group_nodes_into_DFAstates (const regex_t *dfa,
                                       const re_dfastate_t *state,
                                       re_node_set *states_node,
                                       bitset *states_ch);
static int check_node_accept (const regex_t *preg, const re_token_t *node,
                              const re_match_context_t *mctx, int idx);
static reg_errcode_t extend_buffers (re_match_context_t *mctx);

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
    const regex_t *__restrict preg;
    const char *__restrict string;
    size_t nmatch;
    regmatch_t pmatch[];
    int eflags;
{
  reg_errcode_t err;
  int length = strlen (string);
  if (preg->no_sub)
    err = re_search_internal (preg, string, length, 0, length, length, 0,
                              NULL, eflags);
  else
    err = re_search_internal (preg, string, length, 0, length, length, nmatch,
                              pmatch, eflags);
  return err != REG_NOERROR;
}
#ifdef _LIBC
weak_alias (__regexec, regexec)
#endif

/* Entry points for GNU code.  */

/* re_match, re_search, re_match_2, re_search_2

   The former two functions operate on STRING with length LENGTH,
   while the later two operate on concatenation of STRING1 and STRING2
   with lengths LENGTH1 and LENGTH2, respectively.

   re_match() matches the compiled pattern in BUFP against the string,
   starting at index START.

   re_search() first tries matching at index START, then it tries to match
   starting from index START + 1, and so on.  The last start position tried
   is START + RANGE.  (Thus RANGE = 0 forces re_search to operate the same
   way as re_match().)

   The parameter STOP of re_{match,search}_2 specifies that no match exceeding
   the first STOP characters of the concatenation of the strings should be
   concerned.

   If REGS is not NULL, and BUFP->no_sub is not set, the offsets of the match
   and all groups is stroed in REGS.  (For the "_2" variants, the offsets are
   computed relative to the concatenation, not relative to the individual
   strings.)

   On success, re_match* functions return the length of the match, re_search*
   return the position of the start of the match.  Return value -1 means no
   match was found and -2 indicates an internal error.  */

int
re_match (bufp, string, length, start, regs)
    struct re_pattern_buffer *bufp;
    const char *string;
    int length, start;
    struct re_registers *regs;
{
  return re_search_stub (bufp, string, length, start, 0, length, regs, 1);
}
#ifdef _LIBC
weak_alias (__re_match, re_match)
#endif

int
re_search (bufp, string, length, start, range, regs)
    struct re_pattern_buffer *bufp;
    const char *string;
    int length, start, range;
    struct re_registers *regs;
{
  return re_search_stub (bufp, string, length, start, range, length, regs, 0);
}
#ifdef _LIBC
weak_alias (__re_search, re_search)
#endif

int
re_match_2 (bufp, string1, length1, string2, length2, start, regs, stop)
    struct re_pattern_buffer *bufp;
    const char *string1, *string2;
    int length1, length2, start, stop;
    struct re_registers *regs;
{
  return re_search_2_stub (bufp, string1, length1, string2, length2,
                           start, 0, regs, stop, 1);
}
#ifdef _LIBC
weak_alias (__re_match_2, re_match_2)
#endif

int
re_search_2 (bufp, string1, length1, string2, length2, start, range, regs, stop)
    struct re_pattern_buffer *bufp;
    const char *string1, *string2;
    int length1, length2, start, range, stop;
    struct re_registers *regs;
{
  return re_search_2_stub (bufp, string1, length1, string2, length2,
                           start, range, regs, stop, 0);
}
#ifdef _LIBC
weak_alias (__re_search_2, re_search_2)
#endif

static int
re_search_2_stub (bufp, string1, length1, string2, length2, start, range, regs,
                  stop, ret_len)
    struct re_pattern_buffer *bufp;
    const char *string1, *string2;
    int length1, length2, start, range, stop, ret_len;
    struct re_registers *regs;
{
  const char *str;
  int rval;
  int len = length1 + length2;
  int free_str = 0;

  if (BE (length1 < 0 || length2 < 0 || stop < 0, 0))
    return -2;

  /* Concatenate the strings.  */
  if (length2 > 0)
    if (length1 > 0)
      {
        char *s = re_malloc (char, len);

        if (BE (s == NULL, 0))
          return -2;
        memcpy (s, string1, length1);
        memcpy (s + length1, string2, length2);
        str = s;
        free_str = 1;
      }
    else
      str = string2;
  else
    str = string1;

  rval = re_search_stub (bufp, str, len, start, range, stop, regs,
                         ret_len);
  if (free_str)
      re_free ((char *) str);
  return rval;
}

/* The parameters have the same meaning as those of re_search.
   Additional parameters:
   If RET_LEN is nonzero the length of the match is returned (re_match style);
   otherwise the position of the match is returned.  */

static int
re_search_stub (bufp, string, length, start, range, stop, regs, ret_len)
    struct re_pattern_buffer *bufp;
    const char *string;
    int length, start, range, stop, ret_len;
    struct re_registers *regs;
{
  reg_errcode_t result;
  regmatch_t *pmatch;
  int nregs, rval;
  int eflags = 0;

  /* Check for out-of-range.  */
  if (BE (start < 0 || start > length || range < 0, 0))
    return -1;
  if (BE (start + range > length, 0))
    range = length - start;

  eflags |= (bufp->not_bol) ? REG_NOTBOL : 0;
  eflags |= (bufp->not_eol) ? REG_NOTEOL : 0;

  /* Compile fastmap if we haven't yet.  */
  if (range > 0 && bufp->fastmap != NULL && !bufp->fastmap_accurate)
    re_compile_fastmap (bufp);

  if (BE (bufp->no_sub, 0))
    regs = NULL;

  /* We need at least 1 register.  */
  if (regs == NULL)
    nregs = 1;
  else if (BE (bufp->regs_allocated == REGS_FIXED &&
               regs->num_regs < bufp->re_nsub + 1, 0))
    {
      nregs = regs->num_regs;
      if (BE (nregs < 1, 0))
        {
          /* Nothing can be copied to regs.  */
          regs = NULL;
          nregs = 1;
        }
    }
  else
    nregs = bufp->re_nsub + 1;
  pmatch = re_malloc (regmatch_t, nregs);
  if (BE (pmatch == NULL, 0))
    return -2;

  result = re_search_internal (bufp, string, length, start, range, stop,
                               nregs, pmatch, eflags);

  rval = 0;

  /* I hope we needn't fill ther regs with -1's when no match was found.  */
  if (result != REG_NOERROR)
    rval = -1;
  else if (regs != NULL)
    {
      /* If caller wants register contents data back, copy them.  */
      bufp->regs_allocated = re_copy_regs (regs, pmatch, nregs,
                                           bufp->regs_allocated);
      if (BE (bufp->regs_allocated == REGS_UNALLOCATED, 0))
        rval = -2;
    }

  if (BE (rval == 0, 1))
    {
      if (ret_len)
        {
          assert (pmatch[0].rm_so == start);
          rval = pmatch[0].rm_eo - start;
        }
      else
        rval = pmatch[0].rm_so;
    }
  re_free (pmatch);
  return rval;
}

static unsigned
re_copy_regs (regs, pmatch, nregs, regs_allocated)
    struct re_registers *regs;
    regmatch_t *pmatch;
    int nregs, regs_allocated;
{
  int rval = REGS_REALLOCATE;
  int i;
  int need_regs = nregs + 1;
  /* We need one extra element beyond `num_regs' for the `-1' marker GNU code
     uses.  */

  /* Have the register data arrays been allocated?  */
  if (regs_allocated == REGS_UNALLOCATED)
    { /* No.  So allocate them with malloc.  */
      regs->start = re_malloc (regoff_t, need_regs);
      if (BE (regs->start == NULL, 0))
        return REGS_UNALLOCATED;
      regs->end = re_malloc (regoff_t, need_regs);
      if (BE (regs->end == NULL, 0))
        {
          re_free (regs->start);
          return REGS_UNALLOCATED;
        }
      regs->num_regs = need_regs;
    }
  else if (regs_allocated == REGS_REALLOCATE)
    { /* Yes.  If we need more elements than were already
         allocated, reallocate them.  If we need fewer, just
         leave it alone.  */
      if (need_regs > regs->num_regs)
        {
          regs->start = re_realloc (regs->start, regoff_t, need_regs);
          if (BE (regs->start == NULL, 0))
            {
              if (regs->end != NULL)
                re_free (regs->end);
              return REGS_UNALLOCATED;
            }
          regs->end = re_realloc (regs->end, regoff_t, need_regs);
          if (BE (regs->end == NULL, 0))
            {
              re_free (regs->start);
              return REGS_UNALLOCATED;
            }
          regs->num_regs = need_regs;
        }
    }
  else
    {
      assert (regs_allocated == REGS_FIXED);
      /* This function may not be called with REGS_FIXED and nregs too big.  */
      assert (regs->num_regs >= nregs);
      rval = REGS_FIXED;
    }

  /* Copy the regs.  */
  for (i = 0; i < nregs; ++i)
    {
      regs->start[i] = pmatch[i].rm_so;
      regs->end[i] = pmatch[i].rm_eo;
    }
  for ( ; i < regs->num_regs; ++i)
    regs->start[i] = regs->end[i] = -1;

  return rval;
}

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
   Return REG_NOERROR if we find a match, and REG_NOMATCH if not,
   otherwise return the error code.
   Note: We assume front end functions already check ranges.
   (START + RANGE >= 0 && START + RANGE <= LENGTH)  */

static reg_errcode_t
re_search_internal (preg, string, length, start, range, stop, nmatch, pmatch,
                    eflags)
    const regex_t *preg;
    const char *string;
    int length, start, range, stop, eflags;
    size_t nmatch;
    regmatch_t pmatch[];
{
  reg_errcode_t err;
  re_dfa_t *dfa = (re_dfa_t *)preg->buffer;
  re_string_t input;
  int left_lim, right_lim, incr;
  int fl_longest_match, match_first, match_last = -1;
  re_match_context_t mctx;
  char *fastmap = ((preg->fastmap != NULL && preg->fastmap_accurate)
                   ? preg->fastmap : NULL);

  /* Check if the DFA haven't been compiled.  */
  if (BE (preg->used == 0 || dfa->init_state == NULL
          || dfa->init_state_word == NULL || dfa->init_state_nl == NULL
          || dfa->init_state_begbuf == NULL, 0))
    return REG_NOMATCH;

  re_node_set_init_empty (&empty_set);

  /* We must check the longest matching, if nmatch > 0.  */
  fl_longest_match = (nmatch != 0);

  err = re_string_allocate (&input, string, length, dfa->nodes_len + 1,
                            preg->translate, preg->syntax & RE_ICASE);
  if (BE (err != REG_NOERROR, 0))
    return err;
  input.stop = stop;

  err = match_ctx_init (&mctx, eflags, &input, dfa->nbackref * 2);
  if (BE (err != REG_NOERROR, 0))
    return err;

  /* We will log all the DFA states through which the dfa pass,
     if nmatch > 1, or this dfa has "multibyte node", which is a
     back-reference or a node which can accept multibyte character or
     multi character collating element.  */
  if (nmatch > 1 || dfa->has_mb_node)
    {
      mctx.state_log = re_malloc (re_dfastate_t *, dfa->nodes_len + 1);
      if (BE (mctx.state_log == NULL, 0))
        return REG_ESPACE;
    }
  else
    mctx.state_log = NULL;

#ifdef DEBUG
  /* We assume front-end functions already check them.  */
  assert (start + range >= 0 && start + range <= length);
#endif

  match_first = start;
  input.tip_context = ((eflags & REG_NOTBOL) ? CONTEXT_BEGBUF
                       : CONTEXT_NEWLINE | CONTEXT_BEGBUF);

  /* Check incrementally whether of not the input string match.  */
  incr = (range < 0) ? -1 : 1;
  left_lim = (range < 0) ? start + range : start;
  right_lim = (range < 0) ? start : start + range;

  for (;;)
    {
      /* At first get the current byte from input string.  */
      int ch;
      if (MB_CUR_MAX > 1 && (preg->syntax & RE_ICASE || preg->translate))
        {
          /* In this case, we can't determin easily the current byte,
             since it might be a component byte of a multibyte character.
             Then we use the constructed buffer instead.  */
          /* If MATCH_FIRST is out of the valid range, reconstruct the
             buffers.  */
          if (input.raw_mbs_idx + input.valid_len <= match_first)
            re_string_reconstruct (&input, match_first, eflags,
                                   preg->newline_anchor);
          /* If MATCH_FIRST is out of the buffer, leave it as '\0'.
             Note that MATCH_FIRST must not be smaller than 0.  */
          ch = ((match_first >= length) ? 0
                : re_string_byte_at (&input, match_first - input.raw_mbs_idx));
        }
      else
        {
          /* We apply translate/conversion manually, since it is trivial
             in this case.  */
          /* If MATCH_FIRST is out of the buffer, leave it as '\0'.
             Note that MATCH_FIRST must not be smaller than 0.  */
          ch = (match_first < length) ? (unsigned char)string[match_first] : 0;
          /* Apply translation if we need.  */
          ch = preg->translate ? preg->translate[ch] : ch;
          /* In case of case insensitive mode, convert to upper case.  */
          ch = ((preg->syntax & RE_ICASE) && islower (ch)) ? toupper (ch) : ch;
        }

      /* Eliminate inappropriate one by fastmap.  */
      if (preg->can_be_null || fastmap == NULL || fastmap[ch])
        {
          /* Reconstruct the buffers so that the matcher can assume that
             the matching starts from the begining of the buffer.  */
          re_string_reconstruct (&input, match_first, eflags,
                                 preg->newline_anchor);
#ifdef RE_ENABLE_I18N
          /* Eliminate it when it is a component of a multibyte character
             and isn't the head of a multibyte character.  */
          if (MB_CUR_MAX == 1 || re_string_first_byte (&input, 0))
#endif
            {
              /* It seems to be appropriate one, then use the matcher.  */
              /* We assume that the matching starts from 0.  */
              mctx.state_log_top = mctx.nbkref_ents = mctx.max_bkref_len = 0;
              match_last = check_matching (preg, &mctx, 0, fl_longest_match);
              if (match_last != -1)
                {
                  if (BE (match_last == -2, 0))
                    return REG_ESPACE;
                  else
                    break; /* We found a matching.  */
                }
            }
        }
      /* Update counter.  */
      match_first += incr;
      if (match_first < left_lim || right_lim < match_first)
        break;
    }

  /* Set pmatch[] if we need.  */
  if (match_last != -1 && nmatch > 0)
    {
      int reg_idx;

      /* Initialize registers.  */
      for (reg_idx = 0; reg_idx < nmatch; ++reg_idx)
        pmatch[reg_idx].rm_so = pmatch[reg_idx].rm_eo = -1;

      /* Set the points where matching start/end.  */
      pmatch[0].rm_so = 0;
      mctx.match_last = pmatch[0].rm_eo = match_last;

      if (!preg->no_sub && nmatch > 1)
        {
          /* We need the ranges of all the subexpressions.  */
          int halt_node;
          re_dfastate_t *pstate = mctx.state_log[match_last];
#ifdef DEBUG
          assert (mctx.state_log != NULL);
#endif
          halt_node = check_halt_state_context (preg, pstate, &mctx,
                                                match_last);
          err = sift_states_backward (preg, &mctx, halt_node);
          if (BE (err != REG_NOERROR, 0))
            return err;
          err = set_regs (preg, &mctx, nmatch, pmatch, halt_node);
          if (BE (err != REG_NOERROR, 0))
            return err;
        }

      /* At last, add the offset to the each registers, since we slided
         the buffers so that We can assume that the matching starts from 0.  */
      for (reg_idx = 0; reg_idx < nmatch; ++reg_idx)
        if (pmatch[reg_idx].rm_so != -1)
          {
            pmatch[reg_idx].rm_so += match_first;
            pmatch[reg_idx].rm_eo += match_first;
          }
    }

  re_free (mctx.state_log);
  if (dfa->nbackref)
    match_ctx_free (&mctx);
  re_string_destruct (&input);
  return (match_last == -1) ? REG_NOMATCH : REG_NOERROR;
}

/* Acquire an initial state and return it.
   We must select appropriate initial state depending on the context,
   since initial states may have constraints like "\<", "^", etc..  */

static inline re_dfastate_t *
acquire_init_state_context (err, preg, mctx, idx)
     reg_errcode_t *err;
     const regex_t *preg;
     const re_match_context_t *mctx;
     int idx;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;

  *err = REG_NOERROR;
  if (dfa->init_state->has_constraint)
    {
      unsigned int context;
      context =  re_string_context_at (mctx->input, idx - 1, mctx->eflags,
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
        {
          /* It is relatively rare case, then calculate on demand.  */
          return  re_acquire_state_context (err, dfa,
                                            dfa->init_state->entrance_nodes,
                                            context);
        }
      else
        /* Must not happen?  */
        return dfa->init_state;
    }
  else
    return dfa->init_state;
}

/* Check whether the regular expression match input string INPUT or not,
   and return the index where the matching end, return -1 if not match,
   or return -2 in case of an error.
   FL_SEARCH means we must search where the matching starts,
   FL_LONGEST_MATCH means we want the POSIX longest matching.
   Note that the matcher assume that the maching starts from the current
   index of the buffer.  */

static int
check_matching (preg, mctx, fl_search, fl_longest_match)
    const regex_t *preg;
    re_match_context_t *mctx;
    int fl_search, fl_longest_match;
{
  reg_errcode_t err;
  int match = 0;
  int match_last = -1;
  int cur_str_idx = re_string_cur_idx (mctx->input);
  re_dfastate_t *cur_state;

  cur_state = acquire_init_state_context (&err, preg, mctx, cur_str_idx);
  /* An initial state must not be NULL(invalid state).  */
  if (BE (cur_state == NULL, 0))
    return -2;
  if (mctx->state_log != NULL)
    mctx->state_log[cur_str_idx] = cur_state;
  /* If the RE accepts NULL string.  */
  if (cur_state->halt)
    {
      if (!cur_state->has_constraint
          || check_halt_state_context (preg, cur_state, mctx, cur_str_idx))
        {
          if (!fl_longest_match)
            return cur_str_idx;
          else
            {
              match_last = cur_str_idx;
              match = 1;
            }
        }
    }

  while (!re_string_eoi (mctx->input))
    {
      cur_state = transit_state (&err, preg, mctx, cur_state,
                                 fl_search && !match);
      if (cur_state == NULL) /* Reached at the invalid state or an error.  */
        {
          cur_str_idx = re_string_cur_idx (mctx->input);
          if (BE (err != REG_NOERROR, 0))
            return -2;
          if (fl_search && !match)
            {
              /* Restart from initial state, since we are searching
                 the point from where matching start.  */
#ifdef RE_ENABLE_I18N
              if (MB_CUR_MAX == 1
                  || re_string_first_byte (mctx->input, cur_str_idx))
#endif /* RE_ENABLE_I18N */
                cur_state = acquire_init_state_context (&err, preg, mctx,
                                                        cur_str_idx);
              if (BE (cur_state == NULL && err != REG_NOERROR, 0))
                return -2;
              if (mctx->state_log != NULL)
                mctx->state_log[cur_str_idx] = cur_state;
            }
          else if (!fl_longest_match && match)
            break;
          else /* (fl_longest_match && match) || (!fl_search && !match)  */
            {
              if (mctx->state_log == NULL)
                break;
              else
                {
                  int max = mctx->state_log_top;
                  for (; cur_str_idx <= max; ++cur_str_idx)
                    if (mctx->state_log[cur_str_idx] != NULL)
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
              || check_halt_state_context (preg, cur_state, mctx,
                                           re_string_cur_idx (mctx->input)))
            {
              /* We found an appropriate halt state.  */
              match_last = re_string_cur_idx (mctx->input);
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
check_halt_state_context (preg, state, mctx, idx)
    const regex_t *preg;
    const re_dfastate_t *state;
    const re_match_context_t *mctx;
    int idx;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  int i;
  unsigned int context;
#ifdef DEBUG
  assert (state->halt);
#endif
  context = re_string_context_at (mctx->input, idx, mctx->eflags,
                                  preg->newline_anchor);
  for (i = 0; i < state->nodes.nelem; ++i)
    if (check_halt_node_context (dfa, state->nodes.elems[i], context))
      return state->nodes.elems[i];
  return 0;
}

/* Compute the next node to which "NFA" transit from NODE("NFA" is a NFA
   corresponding to the DFA).
   Return the destination node, and update EPS_VIA_NODES, return -1 in case
   of errors.  */

static int
proceed_next_node (preg, mctx, pidx, node, eps_via_nodes)
    const regex_t *preg;
    const re_match_context_t *mctx;
    int *pidx, node;
    re_node_set *eps_via_nodes;
{
  re_dfa_t *dfa = (re_dfa_t *)preg->buffer;
  int i, err, dest_node, cur_entity;
  dest_node = -1;
  cur_entity = ((dfa->nodes[node].type == OP_CONTEXT_NODE)
                ? dfa->nodes[node].opr.ctx_info->entity : node);
  if (IS_EPSILON_NODE (dfa->nodes[node].type))
    {
      int dest_entity = INT_MAX;
      err = re_node_set_insert (eps_via_nodes, node);
      if (BE (err < 0, 0))
        return -1;
      for (i = 0; i < mctx->state_log[*pidx]->nodes.nelem; ++i)
        {
          int candidate, candidate_entity;
          candidate = mctx->state_log[*pidx]->nodes.elems[i];
          candidate_entity = ((dfa->nodes[candidate].type == OP_CONTEXT_NODE)
                              ? dfa->nodes[candidate].opr.ctx_info->entity
                              : candidate);
          if (!re_node_set_contains (dfa->edests + node, candidate))
            if (candidate == candidate_entity
                || !re_node_set_contains (dfa->edests + node, candidate_entity))
              continue;

          /* In order to avoid infinite loop like "(a*)*".  */
          if (cur_entity > candidate_entity
              && re_node_set_contains (eps_via_nodes, candidate))
            continue;

          if (dest_entity > candidate_entity)
            {
              dest_node = candidate;
              dest_entity = candidate_entity;
            }
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

#ifdef RE_ENABLE_I18N
      if (ACCEPT_MB_NODE (type))
        naccepted = check_node_accept_bytes (preg, entity, mctx->input, *pidx);
      else
#endif /* RE_ENABLE_I18N */
      if (type == OP_BACK_REF)
        {
          for (i = 0; i < mctx->nbkref_ents; ++i)
            {
              if (mctx->bkref_ents[i].node == node
                  && mctx->bkref_ents[i].from == *pidx)
                naccepted = mctx->bkref_ents[i].to - *pidx;
            }
          if (naccepted == 0)
            {
              err = re_node_set_insert (eps_via_nodes, node);
              if (BE (err < 0, 0))
                return -1;
              dest_node = dfa->nexts[node];
              if (re_node_set_contains (&mctx->state_log[*pidx]->nodes,
                                        dest_node))
                return dest_node;
              for (i = 0; i < mctx->state_log[*pidx]->nodes.nelem; ++i)
                {
                  dest_node = mctx->state_log[*pidx]->nodes.elems[i];
                  if ((dfa->nodes[dest_node].type == OP_CONTEXT_NODE
                       && (dfa->nexts[node]
                           == dfa->nodes[dest_node].opr.ctx_info->entity)))
                    return dest_node;
                }
            }
        }

      if (naccepted != 0
          || check_node_accept (preg, dfa->nodes + node, mctx, *pidx))
        {
          dest_node = dfa->nexts[node];
          *pidx = (naccepted == 0) ? *pidx + 1 : *pidx + naccepted;
#ifdef DEBUG
          assert (mctx->state_log[*pidx] != NULL);
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

static reg_errcode_t
set_regs (preg, mctx, nmatch, pmatch, last_node)
    const regex_t *preg;
    const re_match_context_t *mctx;
    size_t nmatch;
    regmatch_t *pmatch;
    int last_node;
{
  re_dfa_t *dfa = (re_dfa_t *)preg->buffer;
  int idx, cur_node, real_nmatch;
  re_node_set eps_via_nodes;
#ifdef DEBUG
  assert (nmatch > 1);
  assert (mctx->state_log != NULL);
#endif
  cur_node = dfa->init_node;
  real_nmatch = (nmatch <= preg->re_nsub) ? nmatch : preg->re_nsub + 1;
  re_node_set_init_empty (&eps_via_nodes);
  for (idx = pmatch[0].rm_so; idx <= pmatch[0].rm_eo ;)
    {
      update_regs (dfa, pmatch, cur_node, idx, real_nmatch);
      if (idx == pmatch[0].rm_eo && cur_node == last_node)
        break;

      /* Proceed to next node.  */
      cur_node = proceed_next_node (preg, mctx, &idx, cur_node, &eps_via_nodes);
      if (BE (cur_node < 0, 0))
        return REG_ESPACE;
    }
  re_node_set_free (&eps_via_nodes);
  return REG_NOERROR;
}

static void
update_regs (dfa, pmatch, cur_node, cur_idx, nmatch)
     re_dfa_t *dfa;
     regmatch_t *pmatch;
     int cur_node, cur_idx, nmatch;
{
  int type = dfa->nodes[cur_node].type;
  int reg_num;
  if (type != OP_OPEN_SUBEXP && type != OP_CLOSE_SUBEXP)
    return;
  reg_num = dfa->nodes[cur_node].opr.idx + 1;
  if (reg_num >= nmatch)
    return;
  if (type == OP_OPEN_SUBEXP)
    {
      /* We are at the first node of this sub expression.  */
      pmatch[reg_num].rm_so = cur_idx;
      pmatch[reg_num].rm_eo = -1;
    }
  else if (type == OP_CLOSE_SUBEXP)
    /* We are at the first node of this sub expression.  */
    pmatch[reg_num].rm_eo = cur_idx;
 }

#define NUMBER_OF_STATE 1

/* This function checks the STATE_LOG from the MCTX->match_last to 0
   and sift the nodes in each states according to the following rules.
   Updated state_log will be wrote to STATE_LOG.

   Rules: We throw away the Node `a' in the STATE_LOG[STR_IDX] if...
     1. When STR_IDX == MATCH_LAST(the last index in the state_log):
        If `a' isn't the LAST_NODE and `a' can't epsilon transit to
        the LAST_NODE, we throw away the node `a'.
     2. When 0 <= STR_IDX < MATCH_LAST and `a' accepts
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

static reg_errcode_t
sift_states_backward (preg, mctx, last_node)
    const regex_t *preg;
    const re_match_context_t *mctx;
    int last_node;
{
  reg_errcode_t err;
  re_dfa_t *dfa = (re_dfa_t *)preg->buffer;
  re_node_set state_buf;
  int str_idx = mctx->match_last;
  re_node_set *plog;	/* Points the state_log[str_idx]->nodes  */

#ifdef DEBUG
  assert (mctx->state_log != NULL && mctx->state_log[str_idx] != NULL);
#endif
  err = re_node_set_alloc (&state_buf, NUMBER_OF_STATE);
  if (BE (err != REG_NOERROR, 0))
    return err;
  plog = &mctx->state_log[str_idx]->nodes;

  /* Build sifted state_log[str_idx].  It has the nodes which can epsilon
     transit to the last_node and the last_node itself.  */
  err = re_node_set_intersect (&state_buf, plog, dfa->inveclosures + last_node);
  if (BE (err != REG_NOERROR, 0))
    return err;

  if (mctx->state_log[str_idx] != NULL
      && mctx->state_log[str_idx]->has_backref)
    {
      err = add_epsilon_backreference (dfa, mctx, plog, str_idx, &state_buf);
      if (BE (err != REG_NOERROR, 0))
        return err;
    }

  /* Update state log.  */
  mctx->state_log[str_idx] = re_acquire_state (&err, dfa, &state_buf);
  if (BE (mctx->state_log[str_idx] == NULL && err != REG_NOERROR, 0))
    return err;

  /* Then check each states in the state_log.  */
  while (str_idx > 0)
    {
      int i, j;
      /* Update counters.  */
      re_node_set_empty (&state_buf);
      --str_idx;
      plog = ((mctx->state_log[str_idx] == NULL) ? &empty_set
              : &mctx->state_log[str_idx]->nodes);

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

#ifdef RE_ENABLE_I18N
          /* If the node may accept `multi byte'.  */
          if (ACCEPT_MB_NODE (type))
            naccepted = sift_states_iter_mb (preg, mctx, entity, str_idx,
                                             mctx->match_last);

          /* If the node is a back reference.  */
          else
#endif /* RE_ENABLE_I18N */
          if (type == OP_BACK_REF)
            for (j = 0; j < mctx->nbkref_ents; ++j)
              {
                naccepted = sift_states_iter_bkref (dfa, mctx->state_log,
                                                    mctx->bkref_ents + j,
                                                    prev_node, str_idx,
                                                    mctx->match_last);
                if (naccepted)
                  break;
              }

          if (!naccepted
              && check_node_accept (preg, dfa->nodes + prev_node, mctx,
                                    str_idx)
              && STATE_NODE_CONTAINS (mctx->state_log[str_idx + 1],
                                      dfa->nexts[prev_node]))
            naccepted = 1;

          if (naccepted == 0)
            continue;

          /* `prev_node' may point the entity of the OP_CONTEXT_NODE,
             then we use plog->elems[i] instead.  */
          err = re_node_set_add_intersect (&state_buf, plog,
                                           dfa->inveclosures + prev_node);
          if (BE (err != REG_NOERROR, 0))
            return err;
        }
      if (mctx->state_log[str_idx] != NULL
          && mctx->state_log[str_idx]->has_backref)
        {
          err = add_epsilon_backreference (dfa, mctx, plog, str_idx, &state_buf);
          if (BE (err != REG_NOERROR, 0))
            return err;
        }

      /* Update state_log.  */
      mctx->state_log[str_idx] = re_acquire_state (&err, dfa, &state_buf);
      if (BE (mctx->state_log[str_idx] == NULL && err != REG_NOERROR, 0))
        return err;
    }

  re_node_set_free (&state_buf);
  return REG_NOERROR;
}

/* Helper functions.  */

static inline reg_errcode_t
clean_state_log_if_need (mctx, next_state_log_idx)
    re_match_context_t *mctx;
    int next_state_log_idx;
{
  int top = mctx->state_log_top;

  if (next_state_log_idx >= mctx->input->bufs_len
      || (next_state_log_idx >= mctx->input->valid_len
          && mctx->input->valid_len < mctx->input->len))
    {
      reg_errcode_t err;
      err = extend_buffers (mctx);
      if (BE (err != REG_NOERROR, 0))
        return err;
    }

  if (top < next_state_log_idx)
    {
      memset (mctx->state_log + top + 1, '\0',
              sizeof (re_dfastate_t *) * (next_state_log_idx - top));
      mctx->state_log_top = next_state_log_idx;
    }
  return REG_NOERROR;
}

#ifdef RE_ENABLE_I18N
static int
sift_states_iter_mb (preg, mctx, node_idx, str_idx, max_str_idx)
    const regex_t *preg;
    const re_match_context_t *mctx;
    int node_idx, str_idx, max_str_idx;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  int naccepted;
  /* Check the node can accept `multi byte'.  */
  naccepted = check_node_accept_bytes (preg, node_idx, mctx->input, str_idx);
  if (naccepted > 0 && str_idx + naccepted <= max_str_idx &&
      !STATE_NODE_CONTAINS (mctx->state_log[str_idx + naccepted],
                            dfa->nexts[node_idx]))
    /* The node can't accept the `multi byte', or the
       destination was already throwed away, then the node
       could't accept the current input `multi byte'.   */
    naccepted = 0;
  /* Otherwise, it is sure that the node could accept
     `naccepted' bytes input.  */
  return naccepted;
}
#endif /* RE_ENABLE_I18N */

static int
sift_states_iter_bkref (dfa, state_log, mctx_entry, node_idx, idx, match_last)
    const re_dfa_t *dfa;
    re_dfastate_t **state_log;
    struct re_backref_cache_entry *mctx_entry;
    int node_idx, idx, match_last;
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

static reg_errcode_t
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
          if (j < mctx->nbkref_ents || idx == 0)
            {
              reg_errcode_t err;
              err = re_node_set_add_intersect (state_buf, plog,
                                               dfa->inveclosures + node_idx);
              if (BE (err != REG_NOERROR, 0))
                return err;
              i = 0;
            }
        }
    }
  return REG_NOERROR;
}

/* Functions for state transition.  */

/* Return the next state to which the current state STATE will transit by
   accepting the current input byte, and update STATE_LOG if necessary.
   If STATE can accept a multibyte char/collating element/back reference
   update the destination of STATE_LOG.  */

static re_dfastate_t *
transit_state (err, preg, mctx, state, fl_search)
     reg_errcode_t *err;
     const regex_t *preg;
     re_match_context_t *mctx;
     re_dfastate_t *state;
     int fl_search;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  re_dfastate_t **trtable, *next_state;
  unsigned char ch;

  if (re_string_cur_idx (mctx->input) + 1 >= mctx->input->bufs_len
      || (re_string_cur_idx (mctx->input) + 1 >= mctx->input->valid_len
          && mctx->input->valid_len < mctx->input->len))
    {
      *err = extend_buffers (mctx);
      if (BE (*err != REG_NOERROR, 0))
        return NULL;
    }

  *err = REG_NOERROR;
  if (state == NULL)
    {
      next_state = state;
      re_string_skip_bytes (mctx->input, 1);
    }
  else
    {
#ifdef RE_ENABLE_I18N
      /* If the current state can accept multibyte.  */
      if (state->accept_mb)
        {
          *err = transit_state_mb (preg, state, mctx);
          if (BE (*err != REG_NOERROR, 0))
            return NULL;
        }
#endif /* RE_ENABLE_I18N */

      /* Then decide the next state with the single byte.  */
      if (1)
        {
          /* Use transition table  */
          ch = re_string_fetch_byte (mctx->input);
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
          next_state = transit_state_sb (err, preg, state, fl_search, mctx);
          if (BE (next_state == NULL && err != REG_NOERROR, 0))
            return NULL;
        }
    }

  /* Update the state_log if we need.  */
  if (mctx->state_log != NULL)
    {
      int cur_idx = re_string_cur_idx (mctx->input);
      if (cur_idx > mctx->state_log_top)
        {
          mctx->state_log[cur_idx] = next_state;
          mctx->state_log_top = cur_idx;
        }
      else if (mctx->state_log[cur_idx] == 0)
        {
          mctx->state_log[cur_idx] = next_state;
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
          pstate = mctx->state_log[cur_idx];
          log_nodes = pstate->entrance_nodes;
          if (next_state != NULL)
            {
              table_nodes = next_state->entrance_nodes;
              *err = re_node_set_init_union (&next_nodes, table_nodes,
                                             log_nodes);
              if (BE (*err != REG_NOERROR, 0))
                return NULL;
            }
          else
            next_nodes = *log_nodes;
          /* Note: We already add the nodes of the initial state,
                   then we don't need to add them here.  */

          context = re_string_context_at (mctx->input,
                                          re_string_cur_idx (mctx->input) - 1,
                                          mctx->eflags, preg->newline_anchor);
          next_state = mctx->state_log[cur_idx]
            = re_acquire_state_context (err, dfa, &next_nodes, context);
          /* We don't need to check errors here, since the return value of
             this function is next_state and ERR is already set.  */

          if (table_nodes != NULL)
            re_node_set_free (&next_nodes);
        }
      /* If the next state has back references.  */
      if (next_state != NULL && next_state->has_backref)
        {
          *err = transit_state_bkref (preg, next_state, mctx);
          if (BE (*err != REG_NOERROR, 0))
            return NULL;
          next_state = mctx->state_log[cur_idx];
        }
    }
  return next_state;
}

/* Helper functions for transit_state.  */

/* Return the next state to which the current state STATE will transit by
   accepting the current input byte.  */

static re_dfastate_t *
transit_state_sb (err, preg, state, fl_search, mctx)
     reg_errcode_t *err;
     const regex_t *preg;
     re_dfastate_t *state;
     int fl_search;
     re_match_context_t *mctx;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  re_node_set next_nodes;
  re_dfastate_t *next_state;
  int node_cnt, cur_str_idx = re_string_cur_idx (mctx->input);
  unsigned int context;

  *err = re_node_set_alloc (&next_nodes, state->nodes.nelem + 1);
  if (BE (*err != REG_NOERROR, 0))
    return NULL;
  for (node_cnt = 0; node_cnt < state->nodes.nelem; ++node_cnt)
    {
      int cur_node = state->nodes.elems[node_cnt];
      if (check_node_accept (preg, dfa->nodes + cur_node, mctx, cur_str_idx))
        {
          *err = re_node_set_merge (&next_nodes,
                                    dfa->eclosures + dfa->nexts[cur_node]);
          if (BE (*err != REG_NOERROR, 0))
            return NULL;
        }
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
        {
          *err = re_node_set_merge (&next_nodes,
                                    dfa->init_state->entrance_nodes);
          if (BE (*err != REG_NOERROR, 0))
            return NULL;
        }
    }
  context = re_string_context_at (mctx->input, cur_str_idx, mctx->eflags,
                                  preg->newline_anchor);
  next_state = re_acquire_state_context (err, dfa, &next_nodes, context);
  /* We don't need to check errors here, since the return value of
     this function is next_state and ERR is already set.  */

  re_node_set_free (&next_nodes);
  re_string_skip_bytes (mctx->input, 1);
  return next_state;
}

#ifdef RE_ENABLE_I18N
static reg_errcode_t
transit_state_mb (preg, pstate, mctx)
    const regex_t *preg;
    re_dfastate_t *pstate;
    re_match_context_t *mctx;
{
  reg_errcode_t err;
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
          context = re_string_context_at (mctx->input,
                                          re_string_cur_idx (mctx->input),
                                          mctx->eflags, preg->newline_anchor);
          if (NOT_SATISFY_NEXT_CONSTRAINT (dfa->nodes[cur_node_idx].constraint,
                                        context))
            continue;
          cur_node_idx = dfa->nodes[cur_node_idx].opr.ctx_info->entity;
        }

      /* How many bytes the node can accepts?  */
      if (ACCEPT_MB_NODE (dfa->nodes[cur_node_idx].type))
        naccepted = check_node_accept_bytes (preg, cur_node_idx, mctx->input,
                                             re_string_cur_idx (mctx->input));
      if (naccepted == 0)
        continue;

      /* The node can accepts `naccepted' bytes.  */
      dest_idx = re_string_cur_idx (mctx->input) + naccepted;
      err = clean_state_log_if_need (mctx, dest_idx);
      if (BE (err != REG_NOERROR, 0))
        return err;
#ifdef DEBUG
      assert (dfa->nexts[cur_node_idx] != -1);
#endif
      /* `cur_node_idx' may point the entity of the OP_CONTEXT_NODE,
         then we use pstate->nodes.elems[i] instead.  */
      new_nodes = dfa->eclosures + dfa->nexts[pstate->nodes.elems[i]];

      dest_state = mctx->state_log[dest_idx];
      if (dest_state == NULL)
        dest_nodes = *new_nodes;
      else
        {
          err = re_node_set_init_union (&dest_nodes,
                                        dest_state->entrance_nodes, new_nodes);
          if (BE (err != REG_NOERROR, 0))
            return err;
        }
      context = re_string_context_at (mctx->input, dest_idx - 1, mctx->eflags,
                                      preg->newline_anchor);
      mctx->state_log[dest_idx]
        = re_acquire_state_context (&err, dfa, &dest_nodes, context);
      if (BE (mctx->state_log[dest_idx] == NULL && err != REG_NOERROR, 0))
        return err;
      if (dest_state != NULL)
        re_node_set_free (&dest_nodes);
    }
  return REG_NOERROR;
}
#endif /* RE_ENABLE_I18N */

static reg_errcode_t
transit_state_bkref (preg, pstate, mctx)
    const regex_t *preg;
    re_dfastate_t *pstate;
    re_match_context_t *mctx;
{
  reg_errcode_t err;
  re_dfastate_t **work_state_log;

  work_state_log = re_malloc (re_dfastate_t *,
                              re_string_cur_idx (mctx->input) + 1);
  if (BE (work_state_log == NULL, 0))
    return REG_ESPACE;

  err = transit_state_bkref_loop (preg, &pstate->nodes, work_state_log, mctx);
  re_free (work_state_log);
  return err;
}

/* Caller must allocate `work_state_log'.  */

static reg_errcode_t
transit_state_bkref_loop (preg, nodes, work_state_log, mctx)
    const regex_t *preg;
    re_node_set *nodes;
    re_dfastate_t **work_state_log;
    re_match_context_t *mctx;
{
  reg_errcode_t err;
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  int i, j;
  re_dfastate_t **state_log_bak;
  regmatch_t *cur_regs = re_malloc (regmatch_t, preg->re_nsub + 1);
  int cur_str_idx = re_string_cur_idx (mctx->input);
  if (BE (cur_regs == NULL, 0))
    return REG_ESPACE;

  for (i = 0; i < nodes->nelem; ++i)
    {
      char *buf;
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
          context = re_string_context_at (mctx->input, cur_str_idx,
                                          mctx->eflags, preg->newline_anchor);
          if (NOT_SATISFY_NEXT_CONSTRAINT (node->constraint, context))
            continue;
          subexp_idx = dfa->nodes[node->opr.ctx_info->entity].opr.idx;
        }
      else
        continue;

      /* `node' is a backreference.
         At first, set registers to check the backreference. */
      cur_regs[0].rm_so = 0;
      cur_regs[0].rm_eo = cur_str_idx;
      memcpy (work_state_log, mctx->state_log,
              sizeof (re_dfastate_t *) * (cur_str_idx  + 1));
      mctx->match_last = cur_str_idx;
      state_log_bak = mctx->state_log;
      mctx->state_log = work_state_log;
      sift_states_backward (preg, mctx, node_idx);
      if (!STATE_NODE_CONTAINS (work_state_log[0], dfa->init_node))
        continue;
      for (j = 1; j <= preg->re_nsub; ++j)
        cur_regs[j].rm_so = cur_regs[j].rm_eo = -1;
      set_regs (preg, mctx, subexp_idx + 1, cur_regs, node_idx);
      mctx->state_log = state_log_bak;

      /* Then check that the backreference can match the input string.  */
      subexp_len = cur_regs[subexp_idx].rm_eo - cur_regs[subexp_idx].rm_so;
      if (subexp_len < 0 || cur_str_idx + subexp_len > mctx->input->len)
        continue;

      if (cur_str_idx + subexp_len > mctx->input->valid_len
          && mctx->input->valid_len < mctx->input->len)
        {
          reg_errcode_t err;
          err = extend_buffers (mctx);
          if (BE (err != REG_NOERROR, 0))
            return err;
        }
      buf = re_string_get_buffer (mctx->input);
      if (strncmp (buf + cur_regs[subexp_idx].rm_so, buf + cur_str_idx,
                   subexp_len) != 0)
        continue;

      /* Successfully matched, add a new cache entry.  */
      dest_str_idx = cur_str_idx + subexp_len;
      err = match_ctx_add_entry (mctx, node_idx, cur_str_idx, dest_str_idx);
      if (BE (err != REG_NOERROR, 0))
        return err;
      err = clean_state_log_if_need (mctx, dest_str_idx);
      if (BE (err != REG_NOERROR, 0))
        return err;

      /* And add the epsilon closures (which is `new_dest_nodes') of
         the backreference to appropriate state_log.  */
#ifdef DEBUG
      assert (dfa->nexts[node_idx] != -1);
#endif
      if (node->type == OP_CONTEXT_NODE && subexp_len == 0)
        new_dest_nodes = dfa->nodes[node_idx].opr.ctx_info->bkref_eclosure;
      else
        new_dest_nodes = dfa->eclosures + dfa->nexts[node_idx];
      context = (IS_WORD_CHAR (re_string_byte_at (mctx->input,
                                                  dest_str_idx - 1))
                 ? CONTEXT_WORD : 0);
      dest_state = mctx->state_log[dest_str_idx];

      prev_nelem = ((mctx->state_log[cur_str_idx] == NULL) ? 0
                    : mctx->state_log[cur_str_idx]->nodes.nelem);
      /* Add `new_dest_node' to state_log.  */
      if (dest_state == NULL)
        {
          mctx->state_log[dest_str_idx]
            = re_acquire_state_context (&err, dfa, new_dest_nodes, context);
          if (BE (mctx->state_log[dest_str_idx] == NULL
                  && err != REG_NOERROR, 0))
            return err;
        }
      else
        {
          re_node_set dest_nodes;
          err = re_node_set_init_union (&dest_nodes, dest_state->entrance_nodes,
                                        new_dest_nodes);
          if (BE (err != REG_NOERROR, 0))
            return err;
          mctx->state_log[dest_str_idx]
            = re_acquire_state_context (&err, dfa, &dest_nodes, context);
          if (BE (mctx->state_log[dest_str_idx] == NULL
                  && err != REG_NOERROR, 0))
            return err;
          re_node_set_free (&dest_nodes);
        }

      /* We need to check recursively if the backreference can epsilon
         transit.  */
      if (subexp_len == 0
          && mctx->state_log[cur_str_idx]->nodes.nelem > prev_nelem)
        {
          err = transit_state_bkref_loop (preg, new_dest_nodes, work_state_log,
                                          mctx);
          if (BE (err != REG_NOERROR, 0))
            return err;
        }
    }
  re_free (cur_regs);
  return REG_NOERROR;
}

/* Build transition table for the state.
   Return the new table if succeeded, otherwise return NULL.  */

static re_dfastate_t **
build_trtable (preg, state, fl_search)
    const regex_t *preg;
    const re_dfastate_t *state;
    int fl_search;
{
  reg_errcode_t err;
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
  if (BE (dests_node == NULL || dests_ch == NULL || trtable == NULL, 0))
    return NULL;

  /* At first, group all nodes belonging to `state' into several
     destinations.  */
  ndests = group_nodes_into_DFAstates (preg, state, dests_node, dests_ch);
  if (BE (ndests <= 0, 0))
    {
      re_free (dests_node);
      re_free (dests_ch);
      /* Return NULL in case of an error, trtable otherwise.  */
      return (ndests < 0) ? NULL : trtable;
    }

  dest_states = re_malloc (re_dfastate_t *, ndests);
  dest_states_word = re_malloc (re_dfastate_t *, ndests);
  dest_states_nl = re_malloc (re_dfastate_t *, ndests);
  bitset_empty (acceptable);

  err = re_node_set_alloc (&follows, ndests + 1);
  if (BE (dest_states == NULL || dest_states_word == NULL
          || dest_states_nl == NULL || err != REG_NOERROR, 0))
    return NULL;

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
              err = re_node_set_merge (&follows, dfa->eclosures + next_node);
              if (BE (err != REG_NOERROR, 0))
                return NULL;
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
            {
              err = re_node_set_merge (&follows,
                                       dfa->init_state->entrance_nodes);
              if (BE (err != REG_NOERROR, 0))
                return NULL;
            }
        }
      dest_states[i] = re_acquire_state_context (&err, dfa, &follows, 0);
      if (BE (dest_states[i] == NULL && err != REG_NOERROR, 0))
        return NULL;
      /* If the new state has context constraint,
         build appropriate states for these contexts.  */
      if (dest_states[i]->has_constraint)
        {
          dest_states_word[i] = re_acquire_state_context (&err, dfa, &follows,
                                                          CONTEXT_WORD);
          if (BE (dest_states_word[i] == NULL && err != REG_NOERROR, 0))
            return NULL;
          dest_states_nl[i] = re_acquire_state_context (&err, dfa, &follows,
                                                        CONTEXT_NEWLINE);
          if (BE (dest_states_nl[i] == NULL && err != REG_NOERROR, 0))
            return NULL;
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
  reg_errcode_t err;
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
              err = re_node_set_init_copy (dests_node + ndests, &dests_node[j]);
              if (BE (err != REG_NOERROR, 0))
                return -1;
              ++ndests;
            }

          /* Put the position in the current group. */
          err = re_node_set_insert (&dests_node[j], cur_nodes->elems[i]);
          if (BE (err < 0, 0))
            return -1;

          /* If all characters are consumed, go to next node. */
          if (!not_consumed)
            break;
        }
      /* Some characters remain, create a new group. */
      if (j == ndests)
        {
          bitset_copy (dests_ch[ndests], accepts);
          err = re_node_set_init_1 (dests_node + ndests, cur_nodes->elems[i]);
          if (BE (err != REG_NOERROR, 0))
            return -1;
          ++ndests;
          bitset_empty (accepts);
        }
    }
  return ndests;
}

#ifdef RE_ENABLE_I18N
/* Check how many bytes the node `dfa->nodes[node_idx]' accepts.
   Return the number of the bytes the node accepts.
   STR_IDX is the current index of the input string.

   This function handles the nodes which can accept one character, or
   one collating element like '.', '[a-z]', opposite to the other nodes
   can only accept one byte.  */

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
  int i;
# ifdef _LIBC
  int j;
  uint32_t nrules = _NL_CURRENT_WORD (LC_COLLATE, _NL_COLLATE_NRULES);
# endif /* _LIBC */
  if (elem_len <= 1 && char_len <= 1)
    return 0;
  if (node->type == OP_PERIOD)
    {
      /* '.' accepts any one character except the following two cases.  */
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
# ifdef _LIBC
      const char *pin = re_string_get_buffer (input) + str_idx;
# endif /* _LIBC */
      int match_len = 0;
      wchar_t wc = ((cset->nranges || cset->nchar_classes || cset->nmbchars)
                    ? re_string_wchar_at (input, str_idx) : 0);

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

# ifdef _LIBC
      if (nrules != 0)
        {
          unsigned int in_collseq = 0;
          const int32_t *table, *indirect;
          const char *weights, *extra, *collseqwc;
          int32_t idx;
          /* This #include defines a local function!  */
#  include <locale/weight.h>

          /* match with collating_symbol?  */
          if (cset->ncoll_syms)
            extra = _NL_CURRENT (LC_COLLATE, _NL_COLLATE_SYMB_EXTRAMB);
          for (i = 0; i < cset->ncoll_syms; ++i)
            {
              const char *coll_sym = extra + cset->coll_syms[i];
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
              const unsigned char *cp = (const unsigned char *) pin;
              table = (const int32_t *)
                _NL_CURRENT (LC_COLLATE, _NL_COLLATE_TABLEMB);
              weights = _NL_CURRENT (LC_COLLATE, _NL_COLLATE_WEIGHTMB);
              extra = _NL_CURRENT (LC_COLLATE, _NL_COLLATE_EXTRAMB);
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
        }
      else
# endif /* _LIBC */
        {
          /* match with range expression?  */
#if __GNUC__ >= 2
          wchar_t cmp_buf[] = {L'\0', L'\0', wc, L'\0', L'\0', L'\0'};
#else
          wchar_t cmp_buf[] = {L'\0', L'\0', L'\0', L'\0', L'\0', L'\0'};
          cmp_buf[2] = wc;
#endif
          for (i = 0; i < cset->nranges; ++i)
            {
              cmp_buf[0] = cset->range_starts[i];
              cmp_buf[4] = cset->range_ends[i];
              if (wcscoll (cmp_buf, cmp_buf + 2) <= 0
                  && wcscoll (cmp_buf + 2, cmp_buf + 4) <= 0)
                {
                  match_len = char_len;
                  goto check_node_accept_bytes_match;
                }
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
            return (elem_len > char_len) ? elem_len : char_len;
        }
    }
  return 0;
}

# ifdef _LIBC
static unsigned int
find_collation_sequence_value (mbs, mbs_len)
    const char *mbs;
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
          return collseq[*(unsigned char *) mbs];
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
# endif /* _LIBC */
#endif /* RE_ENABLE_I18N */

/* Check whether the node accepts the byte which is IDX-th
   byte of the INPUT.  */

static int
check_node_accept (preg, node, mctx, idx)
    const regex_t *preg;
    const re_token_t *node;
    const re_match_context_t *mctx;
    int idx;
{
  const re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  const re_token_t *cur_node;
  unsigned char ch;
  if (node->type == OP_CONTEXT_NODE)
    {
      /* The node has constraints.  Check whether the current context
         satisfies the constraints.  */
      unsigned int context = re_string_context_at (mctx->input, idx,
                                                   mctx->eflags,
                                                   preg->newline_anchor);
      if (NOT_SATISFY_NEXT_CONSTRAINT (node->constraint, context))
        return 0;
      cur_node = dfa->nodes + node->opr.ctx_info->entity;
    }
  else
    cur_node = node;

  ch = re_string_byte_at (mctx->input, idx);
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

/* Extend the buffers, if the buffers have run out.  */

static reg_errcode_t
extend_buffers (mctx)
     re_match_context_t *mctx;
{
  reg_errcode_t ret;
  re_string_t *pstr = mctx->input;

  /* Double the lengthes of the buffers.  */
  ret = re_string_realloc_buffers (pstr, pstr->bufs_len * 2);
  if (BE (ret != REG_NOERROR, 0))
    return ret;

  if (mctx->state_log != NULL)
    {
      /* And double the length of state_log.  */
      mctx->state_log = re_realloc (mctx->state_log, re_dfastate_t *,
                                    pstr->bufs_len * 2);
      if (BE (mctx->state_log == NULL, 0))
        return REG_ESPACE;
    }

  /* Then reconstruct the buffers.  */
  if (pstr->icase)
    {
#ifdef RE_ENABLE_I18N
      if (MB_CUR_MAX > 1)
        build_wcs_upper_buffer (pstr);
      else
#endif /* RE_ENABLE_I18N  */
        build_upper_buffer (pstr);
    }
  else
    {
#ifdef RE_ENABLE_I18N
      if (MB_CUR_MAX > 1)
        build_wcs_buffer (pstr);
      else
#endif /* RE_ENABLE_I18N  */
        {
          if (pstr->trans != NULL)
            re_string_translate_buffer (pstr);
          else
            pstr->valid_len = pstr->bufs_len;
        }
    }
  return REG_NOERROR;
}


/* Functions for matching context.  */

static reg_errcode_t
match_ctx_init (mctx, eflags, input, n)
    re_match_context_t *mctx;
    int eflags, n;
    re_string_t *input;
{
  mctx->eflags = eflags;
  mctx->input = input;
  mctx->match_last = -1;
  if (n > 0)
    {
      mctx->bkref_ents = re_malloc (struct re_backref_cache_entry, n);
      if (BE (mctx->bkref_ents == NULL, 0))
        return REG_ESPACE;
    }
  else
    mctx->bkref_ents = NULL;
  mctx->nbkref_ents = 0;
  mctx->abkref_ents = n;
  mctx->max_bkref_len = 0;
  return REG_NOERROR;
}

static void
match_ctx_free (mctx)
    re_match_context_t *mctx;
{
  re_free (mctx->bkref_ents);
}

/* Add a new backreference entry to the cache.  */

static reg_errcode_t
match_ctx_add_entry (mctx, node, from, to)
    re_match_context_t *mctx;
    int node, from, to;
{
  if (mctx->nbkref_ents >= mctx->abkref_ents)
    {
      mctx->bkref_ents = re_realloc (mctx->bkref_ents,
                                     struct re_backref_cache_entry,
                                     mctx->abkref_ents * 2);
      if (BE (mctx->bkref_ents == NULL, 0))
        return REG_ESPACE;
      memset (mctx->bkref_ents + mctx->nbkref_ents, '\0',
             sizeof (struct re_backref_cache_entry) * mctx->abkref_ents);
      mctx->abkref_ents *= 2;
    }
  mctx->bkref_ents[mctx->nbkref_ents].node = node;
  mctx->bkref_ents[mctx->nbkref_ents].from = from;
  mctx->bkref_ents[mctx->nbkref_ents++].to = to;
  if (mctx->max_bkref_len < to - from)
    mctx->max_bkref_len = to - from;
  return REG_NOERROR;
}
