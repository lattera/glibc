/* Internal header for proving correct grouping in strings of numbers.
Copyright (C) 1995 Free Software Foundation, Inc.
Contributed by Ulrich Drepper.

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

#include <limits.h>

#ifndef MAX
#define MAX(a,b)	({ typeof(a) _a = (a); typeof(b) _b = (b); \
			   _a > _b ? _a : _b; })
#endif

/* Find the maximum prefix of the string between BEGIN and END which
   satisfies the grouping rules.  It is assumed that at least one digit
   follows BEGIN directly.  */

static inline const char *
correctly_grouped_prefix (const char *begin, const char *end,
			  wchar_t thousands, const char *grouping)
{
  if (! grouping)
    return end;

  while (end > begin)
    {
      const char *cp = end - 1;
      const char *gp = grouping;

      /* Check first group.  */
      while (cp >= begin && (wchar_t) *cp != thousands)
	--cp;

      if (end - cp == (int) *gp + 1)
	{
	  /* This group matches the specification.  */

	  const char *new_end;

	  if (cp < begin)
	    /* There is just one complete group.  We are done.  */
	    return end;

	  /* CP points to a thousands separator character.  The preceding
	     remainder of the string from BEGIN to NEW_END is the part we
	     will consider if there is a grouping error in this trailing
	     portion from CP to END.  */
	  new_end = cp - 1;

	  /* Loop while the grouping is correct.  */
	  while (1)
	    {
	      /* Get the next grouping rule.  */
	      ++gp;
	      if (*gp == 0)
		/* If end is reached use last rule.  */ 
	        --gp;

	      /* Skip the thousands separator.  */
	      --cp;

	      if (*gp == CHAR_MAX || *gp < 0)
	        {
	          /* No more thousands separators are allowed to follow.  */
	          while (cp >= begin && (wchar_t) *cp != thousands)
		    --cp;

	          if (cp < begin)
		    /* OK, only digits followed.  */
		    return end;
	        }
	      else
	        {
		  /* Check the next group.  */
	          const char *group_end = cp;

		  while (cp >= begin && (wchar_t) *cp != thousands)
		    --cp;

		  if (cp < begin && group_end - cp <= (int) *gp)
		    /* Final group is correct.  */
		    return end;

		  if (cp < begin || group_end - cp != (int) *gp)
		    /* Incorrect group.  Punt.  */
		    break;
		}
	    }

	  /* The trailing portion of the string starting at NEW_END
	     contains a grouping error.  So we will look for a correctly
	     gouped number in the preceding portion instead.  */
	  end = new_end;
	}
      else
	{
	  /* Even the first group was wrong; determine maximum shift.  */
	  if (end - cp > (int) *gp + 1)
	    end = cp + (int) *gp + 1;
	  else if (cp < begin)
	    /* This number does not fill the first group, but is correct.  */
	    return end;
	  else
	    /* CP points to a thousands seperator character.  */
	    end = cp;
	}
    }

  return MAX (begin, end);
} 
