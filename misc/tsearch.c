/* Copyright (C) 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.

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

/* Tree search generalized from Knuth (6.2.2) Algorithm T just like
   the AT&T man page says.
  
   The node_t structure is for internal use only, lint doesn't grok it.
  
   Written by reading the System V Interface Definition, not the code.
  
   Totally public domain.  */
/*LINTLIBRARY*/

#include <stdlib.h>
#include <search.h>

/* This routine is not very bad.  It makes many assumptions about
   the compiler. It assumpts that the first field in node must be
   the "key" field, which points to the datum. It is a very trick
   stuff. H.J.  */

typedef struct node_t
{
  const void *key;
  struct node_t *left;
  struct node_t *right;
}
node;

/* Prototype fpr local function.  */
static void trecurse __P ((const void *vroot, __action_fn_t action, int level));


/* find or insert datum into search tree.
char 	*key;		 key to be located
node	**rootp;	 address of tree root
int	(*compar)();	 ordering function
*/
void *
tsearch (key, vrootp, compar)
     const void *key;
     void **vrootp;
     __compar_fn_t compar;
{
  node *q;
  node **rootp = (node **) vrootp;

  if (rootp == NULL)
    return NULL;

  while (*rootp != NULL)		/* Knuth's T1: */
    {
      int r;

      r = (*compar) (key, (*rootp)->key);
      if (r == 0)			/* T2: */
	return *rootp;			/* we found it! */
      rootp = (r < 0)
	      ? &(*rootp)->left		/* T3: follow left branch */
	      : &(*rootp)->right;	/* T4: follow right branch */
    }

  q = (node *) malloc (sizeof (node));	/* T5: key not found */
  if (q != NULL)			/* make new node */
    {
      *rootp = q;			/* link new node to old */
      q->key = key;			/* initialize new node */
      q->left = q->right = NULL;
    }

  return q;
}


void *
tfind (key, vrootp, compar)
     const void *key;
     const void **vrootp;
     __compar_fn_t compar;
{
  node **rootp = (node **) vrootp;

  if (rootp == NULL)
    return NULL;

  while (*rootp != NULL)		/* Knuth's T1: */
    {
      int r;

      r = (*compar)(key, (*rootp)->key);
      if (r == 0)			/* T2: */
	return *rootp;			/* we found it! */

      rootp = (r < 0)
	      ? &(*rootp)->left		/* T3: follow left branch */
	      : &(*rootp)->right;	/* T4: follow right branch */
    }
    return NULL;
}


/* delete node with given key
char	*key;		key to be deleted
node	**rootp;	address of the root of tree
int	(*compar)();	comparison function
*/
void *
tdelete (key, vrootp, compar)
     const void *key;
     void **vrootp;
     __compar_fn_t compar;
{
  node *p;
  node *q;
  node *r;
  int cmp;
  node **rootp = (node **) vrootp;

  if (rootp == NULL || (p = *rootp) == NULL)
    return NULL;

  while ((cmp = (*compar) (key, (*rootp)->key)) != 0)
    {
      p = *rootp;
      rootp = (cmp < 0)
	      ? &(*rootp)->left		/* follow left branch */
	      : &(*rootp)->right;	/* follow right branch */
      if (*rootp == NULL)
	return NULL;			/* key not found */
    }

  r = (*rootp)->right;			/* D1: */
  q = (*rootp)->left;
  if (q == NULL)			/* Left NULL? */
    q = r;
  else if (r != NULL)			/* Right link is NULL? */
    {
      if (r->left == NULL)		/* D2: Find successor */
	{
	  r->left = q;
	  q = r;
	}
      else
	{				/* D3: Find (struct node_t *)0 link */
	  for (q = r->left; q->left != NULL; q = r->left)
	    r = q;
	  r->left = q->right;
	  q->left = (*rootp)->left;
	  q->right = (*rootp)->right;
	}
    }
  free ((struct node_t *) *rootp);	/* D4: Free node */
  *rootp = q;				/* link parent to new node */
  return p;
}


/* Walk the nodes of a tree
node	*root;		Root of the tree to be walked
void	(*action)();	Function to be called at each node
int	level;
*/
static void
trecurse (vroot, action, level)
     const void *vroot;
     __action_fn_t action;
     int level;
{
  node *root = (node *) vroot;

  if (root->left == NULL && root->right == NULL)
    (*action) (root, leaf, level);
  else
    {
      (*action) (root, preorder, level);
      if (root->left != NULL)
	trecurse (root->left, action, level + 1);
      (*action) (root, postorder, level);
      if (root->right != NULL)
	trecurse (root->right, action, level + 1);
      (*action) (root, endorder, level);
    }
}


/* void twalk(root, action)	Walk the nodes of a tree 
node	*root;			Root of the tree to be walked
void	(*action)();		Function to be called at each node
PTR
*/
void
twalk (vroot, action)
     const void *vroot;
     __action_fn_t action;
{
  const node *root = (node *) vroot;

  if (root != NULL && action != NULL)
    trecurse (root, action, 0);
}
