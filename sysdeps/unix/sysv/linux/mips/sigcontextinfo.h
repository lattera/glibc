/* Copyright (C) 2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 2000.

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


#define SIGCONTEXT unsigned long _code, struct sigcontext
#define SIGCONTEXT_EXTRA_ARGS _code,
#define GET_PC(ctx)	((void *) ctx->sc_pc)
#define GET_FRAME(ctx)	((void *) ctx->sc_regs[30])
#define GET_STACK(ctx)	((void *) ctx->sc_regs[29])
