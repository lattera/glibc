/* Provide access to the collection of available transformation modules.
   Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <limits.h>
#include <search.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <bits/libc-lock.h>

#include <ldsodefs.h>
#include <gconv_int.h>


/* Simple data structure for alias mapping.  We have two names, `from'
   and `to'.  */
void *__gconv_alias_db;

/* Array with available modules.  */
struct gconv_module *__gconv_modules_db;

/* We modify global data.   */
__libc_lock_define_initialized (static, lock)


/* Function for searching alias.  */
int
__gconv_alias_compare (const void *p1, const void *p2)
{
  struct gconv_alias *s1 = (struct gconv_alias *) p1;
  struct gconv_alias *s2 = (struct gconv_alias *) p2;
  return __strcasecmp (s1->fromname, s2->fromname);
}


/* To search for a derivation we create a list of intermediate steps.
   Each element contains a pointer to the element which precedes it
   in the derivation order.  */
struct derivation_step
{
  const char *result_set;
  size_t result_set_len;
  int cost_lo;
  int cost_hi;
  struct gconv_module *code;
  struct derivation_step *last;
  struct derivation_step *next;
};

#define NEW_STEP(result, hi, lo, module, last_mod) \
  ({ struct derivation_step *newp = alloca (sizeof (struct derivation_step)); \
     newp->result_set = result;						      \
     newp->result_set_len = strlen (result);				      \
     newp->cost_hi = hi;						      \
     newp->cost_lo = lo;						      \
     newp->code = module;						      \
     newp->last = last_mod;						      \
     newp->next = NULL;							      \
     newp; })


/* If a specific transformation is used more than once we should not need
   to start looking for it again.  Instead cache each successful result.  */
struct known_derivation
{
  const char *from;
  const char *to;
  struct gconv_step *steps;
  size_t nsteps;
};

/* Compare function for database of found derivations.  */
static int
derivation_compare (const void *p1, const void *p2)
{
  struct known_derivation *s1 = (struct known_derivation *) p1;
  struct known_derivation *s2 = (struct known_derivation *) p2;
  int result;

  result = strcmp (s1->from, s2->from);
  if (result == 0)
    result = strcmp (s1->to, s2->to);
  return result;
}

/* The search tree for known derivations.  */
static void *known_derivations;

/* Look up whether given transformation was already requested before.  */
static int
internal_function
derivation_lookup (const char *fromset, const char *toset,
		   struct gconv_step **handle, size_t *nsteps)
{
  struct known_derivation key = { fromset, toset, NULL, 0 };
  struct known_derivation **result;

  result = __tfind (&key, &known_derivations, derivation_compare);

  if (result == NULL)
    return GCONV_NOCONV;

  *handle = (*result)->steps;
  *nsteps = (*result)->nsteps;

  /* Please note that we return GCONV_OK even if the last search for
     this transformation was unsuccessful.  */
  return GCONV_OK;
}

/* Add new derivation to list of known ones.  */
static void
internal_function
add_derivation (const char *fromset, const char *toset,
		struct gconv_step *handle, size_t nsteps)
{
  struct known_derivation *new_deriv;
  size_t fromset_len = strlen (fromset) + 1;
  size_t toset_len = strlen (toset) + 1;

  new_deriv = (struct known_derivation *)
    malloc (sizeof (struct known_derivation) + fromset_len + toset_len);
  if (new_deriv != NULL)
    {
      new_deriv->from = memcpy (new_deriv + 1, fromset, fromset_len);
      new_deriv->to = memcpy ((char *) new_deriv->from + fromset_len,
			      toset, toset_len);

      new_deriv->steps = handle;
      new_deriv->nsteps = nsteps;

      __tsearch (new_deriv, &known_derivations, derivation_compare);
    }
  /* Please note that we don't complain if the allocation failed.  This
     is not tragically but in case we use the memory debugging facilities
     not all memory will be freed.  */
}

static void
free_derivation (void *p)
{
  struct known_derivation *deriv = (struct known_derivation *) p;
  size_t cnt;

  for (cnt = 0; cnt < deriv->nsteps; ++cnt)
    if (deriv->steps[cnt].end_fct)
      _CALL_DL_FCT (deriv->steps[cnt].end_fct, (&deriv->steps[cnt]));

  free ((struct gconv_step *) deriv->steps);
  free (deriv);
}


static int
internal_function
gen_steps (struct derivation_step *best, const char *toset,
	   const char *fromset, struct gconv_step **handle, size_t *nsteps)
{
  size_t step_cnt = 0;
  struct gconv_step *result;
  struct derivation_step *current;
  int status = GCONV_NOMEM;

  /* First determine number of steps.  */
  for (current = best; current->last != NULL; current = current->last)
    ++step_cnt;

  result = (struct gconv_step *) malloc (sizeof (struct gconv_step)
					 * step_cnt);
  if (result != NULL)
    {
      int failed = 0;

      status = GCONV_OK;
      *nsteps = step_cnt;
      current = best;
      while (step_cnt-- > 0)
	{
	  result[step_cnt].from_name = (step_cnt == 0
					? __strdup (fromset)
					: current->last->result_set);
	  result[step_cnt].to_name = (step_cnt + 1 == *nsteps
				      ? __strdup (current->result_set)
				      : result[step_cnt + 1].from_name);

#ifndef STATIC_GCONV
	  if (current->code->module_name[0] == '/')
	    {
	      /* Load the module, return handle for it.  */
	      struct gconv_loaded_object *shlib_handle =
		__gconv_find_shlib (current->code->module_name);

	      if (shlib_handle == NULL)
		{
		  failed = 1;
		  break;
		}

	      result[step_cnt].shlib_handle = shlib_handle;
	      result[step_cnt].modname = shlib_handle->name;
	      result[step_cnt].counter = 0;
	      result[step_cnt].fct = shlib_handle->fct;
	      result[step_cnt].init_fct = shlib_handle->init_fct;
	      result[step_cnt].end_fct = shlib_handle->end_fct;
	    }
	  else
#endif
	    /* It's a builtin transformation.  */
	    __gconv_get_builtin_trans (current->code->module_name,
				       &result[step_cnt]);

	  /* Call the init function.  */
	  if (result[step_cnt].init_fct != NULL)
	     {
	       status = _CALL_DL_FCT (result[step_cnt].init_fct,
				      (&result[step_cnt]));

	       if (status != GCONV_OK)
		 {
		   failed = 1;
		   /* Make sure we unload this modules.  */
		   --step_cnt;
		   break;
		 }
	     }

	  current = current->last;
	}

      if (failed != 0)
	{
	  /* Something went wrong while initializing the modules.  */
	  while (++step_cnt < *nsteps)
	    {
	      if (result[step_cnt].end_fct != NULL)
		_CALL_DL_FCT (result[step_cnt].end_fct, (&result[step_cnt]));
#ifndef STATIC_GCONV
	      __gconv_release_shlib (result[step_cnt].shlib_handle);
#endif
	    }
	  free (result);
	  *nsteps = 0;
	  *handle = NULL;
	  if (status == GCONV_OK)
	    status = GCONV_NOCONV;
	}
      else
	*handle = result;
    }
  else
    {
      *nsteps = 0;
      *handle = NULL;
    }

  return status;
}


/* The main function: find a possible derivation from the `fromset' (either
   the given name or the alias) to the `toset' (again with alias).  */
static int
internal_function
find_derivation (const char *toset, const char *toset_expand,
		 const char *fromset, const char *fromset_expand,
		 struct gconv_step **handle, size_t *nsteps)
{
  __libc_lock_define_initialized (static, lock)
  struct derivation_step *first, *current, **lastp, *solution = NULL;
  int best_cost_hi = INT_MAX;
  int best_cost_lo = INT_MAX;
  int result;

  result = derivation_lookup (fromset_expand ?: fromset, toset_expand ?: toset,
			      handle, nsteps);
  if (result == GCONV_OK)
    return result;

  __libc_lock_lock (lock);

  /* There is a small chance that this derivation is meanwhile found.  This
     can happen if in `find_derivation' we look for this derivation, didn't
     find it but at the same time another thread looked for this derivation. */
  result = derivation_lookup (fromset_expand ?: fromset, toset_expand ?: toset,
			      handle, nsteps);
  if (result == GCONV_OK)
    {
      __libc_lock_unlock (lock);
      return result;
    }

  /* For now we use a simple algorithm with quadratic runtime behaviour.
     The task is to match the `toset' with any of the available rules,
     starting from FROMSET.  */
  if (fromset_expand != NULL)
    {
      first = NEW_STEP (fromset_expand, 0, 0, NULL, NULL);
      first->next = NEW_STEP (fromset, 0, 0, NULL, NULL);
      lastp = &first->next->next;
    }
  else
    {
      first = NEW_STEP (fromset, 0, 0, NULL, NULL);
      lastp = &first->next;
    }

  for (current = first; current != NULL; current = current->next)
    {
      /* Now match all the available module specifications against the
         current charset name.  If any of them matches check whether
         we already have a derivation for this charset.  If yes, use the
         one with the lower costs.  Otherwise add the new charset at the
         end.

	 The module database is organized in a tree form which allows to
	 search for prefixes.  So we search for the first entry with a
	 matching prefix and any other matching entry can be found from
	 this place.  */
      struct gconv_module *node = __gconv_modules_db;

      /* Maybe it is not necessary anymore to look for a solution for
	 this entry since the cost is already as high (or heigher) as
	 the cost for the best solution so far.  */
      if (current->cost_hi > best_cost_hi
	  || (current->cost_hi == best_cost_hi
	      && current->cost_lo >= best_cost_lo))
	continue;

      while (node != NULL)
	{
	  int cmpres = strncmp (current->result_set, node->from_constpfx,
				MIN (current->result_set_len,
				     node->from_constpfx_len));

	  if (cmpres == 0)
	    {
	      /* Walk through the list of modules with this prefix and
		 try to match the name.  */
	      struct gconv_module *runp;

	      if (current->result_set_len < node->from_constpfx_len)
		/* Cannot possibly match.  */
		break;

	      /* Check all the modules with this prefix.  */
	      runp = node;
	      do
		{
		  const char *result_set = NULL;

		  if (runp->from_pattern == NULL)
		    {
		      /* This is a simple entry and therefore we have a
			 found an matching entry if the strings are really
			 equal.  */
		      if (current->result_set_len == runp->from_constpfx_len)
			{
			  if (strcmp (runp->to_string, "-") == 0)
			    result_set = toset_expand ?: toset;
			  else
			    result_set = runp->to_string;
			}
		    }
		  else
		    {
		      /* Compile the regular expression if necessary.  */
		      if (runp->from_regex == NULL)
			{
			  if (__regcomp (&runp->from_regex_mem,
					 runp->from_pattern,
					 REG_EXTENDED | REG_ICASE) != 0)
			    /* Something is wrong.  Remember this.  */
			    runp->from_regex = (regex_t *) -1L;
			  else
			    runp->from_regex = &runp->from_regex_mem;
			}

		      if (runp->from_regex != (regex_t *) -1L)
			{
			  regmatch_t match[4];

			  /* Try to match the regular expression.  */
			  if (__regexec (runp->from_regex, current->result_set,
					 4, match, 0) == 0
			      && match[0].rm_so == 0
			      && current->result_set[match[0].rm_eo] == '\0')
			    {
			      /* At least the whole <from> string is matched.
				 We must now match sed-like possible
				 subexpressions from the match to the
				 toset expression.  */
#define ENSURE_LEN(LEN) \
  if (wp + (LEN) >= constr + len - 1)					      \
    {									      \
      char *newp = alloca (len += 128);					      \
      wp = __mempcpy (newp, constr, wp - constr);			      \
      constr = newp;							      \
    }
			      size_t len = 128;
			      char *constr = alloca (len);
			      char *wp = constr;
			      const char *cp = runp->to_string;

			      while (*cp != '\0')
				{
				  if (*cp != '\\')
				    {
				      ENSURE_LEN (1);
				      *wp++ = *cp++;
				    }
				  else if (cp[1] == '\0')
				    /* Backslash at end of string.  */
				    break;
				  else
				    {
				      ++cp;
				      if (*cp == '\\')
					{
					  *wp++ = *cp++;
					  ENSURE_LEN (1);
					}
				      else if (*cp < '1' || *cp > '3')
					break;
				      else
					{
					  int idx = *cp - '0';
					  if (match[idx].rm_so == -1)
					    /* No match.  */
					    break;

					  ENSURE_LEN (match[idx].rm_eo
						      - match[idx].rm_so);
					  wp = __mempcpy (wp,
							  &current->result_set[match[idx].rm_so],
							  match[idx].rm_eo
							  - match[idx].rm_so);
					  ++cp;
					}
				    }
				}
			      if (*cp == '\0' && wp != constr)
				{
				  /* Terminate the constructed string.  */
				  *wp = '\0';
				  result_set = constr;
				}
			    }
			}
		    }

		  if (result_set != NULL)
		    {
		      int cost_hi = runp->cost_hi + current->cost_hi;
		      int cost_lo = runp->cost_lo + current->cost_lo;
		      struct derivation_step *step;

		      /* We managed to find a derivation.  First see whether
			 this is what we are looking for.  */
		      if (__strcasecmp (result_set, toset) == 0
			  || (toset_expand != NULL
			      && __strcasecmp (result_set, toset_expand) == 0))
			{
			  if (solution == NULL || cost_hi < best_cost_hi
			      || (cost_hi == best_cost_hi
				  && cost_lo < best_cost_lo))
			    {
			      best_cost_hi = cost_hi;
			      best_cost_lo = cost_lo;
			    }

			  /* Append this solution to list.  */
			  if (solution == NULL)
			    solution = NEW_STEP (result_set, 0, 0, runp,
						 current);
			  else
			    {
			      while (solution->next != NULL)
				solution = solution->next;

			      solution->next = NEW_STEP (result_set, 0, 0,
							 runp, current);
			    }
			}
		      else if (cost_hi < best_cost_hi
			       || (cost_hi == best_cost_hi
				   && cost_lo < best_cost_lo))
			{
			  /* Append at the end if there is no entry with
			     this name.  */
			  for (step = first; step != NULL; step = step->next)
			    if (__strcasecmp (result_set, step->result_set)
				== 0)
			      break;

			  if (step == NULL)
			    {
			      *lastp = NEW_STEP (result_set,
						 cost_hi, cost_lo,
						 runp, current);
			      lastp = &(*lastp)->next;
			    }
			  else if (step->cost_hi > cost_hi
				   || (step->cost_hi == cost_hi
				       && step->cost_lo > cost_lo))
			    {
			      step->code = runp;
			      step->last = current;

			      /* Update the cost for all steps.  */
			      for (step = first; step != NULL;
				   step = step->next)
				{
				  struct derivation_step *back;

				  if (step->code == NULL)
				    /* This is one of the entries we started
				       from.  */
				    continue;

				  step->cost_hi = step->code->cost_hi;
				  step->cost_lo = step->code->cost_lo;

				  for (back = step->last; back->code != NULL;
				       back = back->last)
				    {
				      step->cost_hi += back->code->cost_hi;
				      step->cost_lo += back->code->cost_lo;
				    }
				}

			      for (step = solution; step != NULL;
				   step = step->next)
				{
				  step->cost_hi = (step->code->cost_hi
						   + step->last->cost_hi);
				  step->cost_lo = (step->code->cost_lo
						   + step->last->cost_lo);

				  if (step->cost_hi < best_cost_hi
				      || (step->cost_hi == best_cost_hi
					  && step->cost_lo < best_cost_lo))
				    {
				      solution = step;
				      best_cost_hi = step->cost_hi;
				      best_cost_lo = step->cost_lo;
				    }
				}
			    }
			}
		    }

		  runp = runp->same;
		}
	      while (runp != NULL);

	      if (current->result_set_len == node->from_constpfx_len)
		break;

	      node = node->matching;
	    }
	  else if (cmpres < 0)
	    node = node->left;
	  else
	    node = node->right;
	}
    }

  if (solution != NULL)
    /* We really found a way to do the transformation.  Now build a data
       structure describing the transformation steps.*/
    result = gen_steps (solution, toset_expand ?: toset,
			fromset_expand ?: fromset, handle, nsteps);
  else
    {
      /* We haven't found a transformation.  Clear the result values.  */
      *handle = NULL;
      *nsteps = 0;
    }

  /* Add result in any case to list of known derivations.  */
  add_derivation (fromset_expand ?: fromset, toset_expand ?: toset,
		  *handle, *nsteps);

  __libc_lock_unlock (lock);

  return result;
}


int
internal_function
__gconv_find_transform (const char *toset, const char *fromset,
			struct gconv_step **handle, size_t *nsteps)
{
  __libc_once_define (static, once);
  const char *fromset_expand = NULL;
  const char *toset_expand = NULL;
  int result;

  /* Ensure that the configuration data is read.  */
  __libc_once (once, __gconv_read_conf);

  /* Acquire the lock.  */
  __libc_lock_lock (lock);

  /* If we don't have a module database return with an error.  */
  if (__gconv_modules_db == NULL)
    {
      __libc_lock_unlock (lock);
      return GCONV_NOCONV;
    }

  /* See whether the names are aliases.  */
  if (__gconv_alias_db != NULL)
    {
      struct gconv_alias key;
      struct gconv_alias **found;

      key.fromname = fromset;
      found = __tfind (&key, &__gconv_alias_db, __gconv_alias_compare);
      fromset_expand = found != NULL ? (*found)->toname : NULL;

      key.fromname = toset;
      found = __tfind (&key, &__gconv_alias_db, __gconv_alias_compare);
      toset_expand = found != NULL ? (*found)->toname : NULL;
    }

  result = find_derivation (toset, toset_expand, fromset, fromset_expand,
			    handle, nsteps);

#ifndef STATIC_GCONV
  /* Increment the user counter.  */
  if (result == GCONV_OK)
    {
      size_t cnt = *nsteps;
      struct gconv_step *steps = *handle;

      do
	if (steps[--cnt].counter++ == 0)
	  {
	    steps[cnt].shlib_handle =
	      __gconv_find_shlib (steps[cnt].modname);
	    if (steps[cnt].shlib_handle == NULL)
	      {
		/* Oops, this is the second time we use this module (after
		   unloading) and this time loading failed!?  */
		while (++cnt < *nsteps)
		  __gconv_release_shlib (steps[cnt].shlib_handle);
		result = GCONV_NOCONV;
		break;
	      }
	  }
      while (cnt > 0);
    }
#endif

  /* Release the lock.  */
  __libc_lock_unlock (lock);

  /* The following code is necessary since `find_derivation' will return
     GCONV_OK even when no derivation was found but the same request
     was processed before.  I.e., negative results will also be cached.  */
  return (result == GCONV_OK
	  ? (*handle == NULL ? GCONV_NOCONV : GCONV_OK)
	  : result);
}


/* Release the entries of the modules list.  */
int
internal_function
__gconv_close_transform (struct gconv_step *steps, size_t nsteps)
{
  int result = GCONV_OK;

#ifndef STATIC_GCONV
  /* Acquire the lock.  */
  __libc_lock_lock (lock);

  while (nsteps-- > 0)
    if (steps[nsteps].shlib_handle != NULL
	&& --steps[nsteps].counter == 0)
      {
	result = __gconv_release_shlib (steps[nsteps].shlib_handle);
	if (result != GCONV_OK)
	  break;
	steps[nsteps].shlib_handle = NULL;
      }

  /* Release the lock.  */
  __libc_lock_unlock (lock);
#endif

  return result;
}


/* Free the modules mentioned.  */
static void
internal_function
free_modules_db (struct gconv_module *node)
{
  if (node->left != NULL)
    free_modules_db (node->left);
  if (node->right != NULL)
    free_modules_db (node->right);
  if (node->same != NULL)
    free_modules_db (node->same);
  do
    {
      struct gconv_module *act = node;
      node = node->matching;
      free (act);
    }
  while (node != NULL);
}


/* Free all resources if necessary.  */
static void __attribute__ ((unused))
free_mem (void)
{
  if (__gconv_alias_db != NULL)
    __tdestroy (__gconv_alias_db, free);

  free_modules_db (__gconv_modules_db);

  if (known_derivations != NULL)
    __tdestroy (known_derivations, free_derivation);
}

text_set_element (__libc_subfreeres, free_mem);
