/* Copyright (C) 1991 Free Software Foundation, Inc.
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
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef	_EXIT_H

struct exit_function
  {
    enum { ef_free, ef_on, ef_at } flavor;	/* `ef_free' MUST be zero!  */
    union
      {
	void EXFUN((*at), (NOARGS));
	struct
	  {
	    void EXFUN((*fn), (int status, PTR arg));
	    PTR arg;
	  } on;
      } func;
  };
struct exit_function_list
  {
    struct exit_function_list *next;
    size_t idx;
    struct exit_function fns[32];
  };
extern struct exit_function_list *__exit_funcs;

extern struct exit_function *EXFUN(__new_exitfn, (NOARGS));

#endif	/* exit.h  */
