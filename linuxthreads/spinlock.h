/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr) and          */
/* Richard Henderson (rth@tamu.edu)                                     */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

/* Spin locks */

extern void __pthread_acquire(int * spinlock);

static inline void acquire(int * spinlock)
{
  if (testandset(spinlock)) __pthread_acquire(spinlock);
}

static inline void release(int * spinlock)
{
#ifndef RELEASE
  *spinlock = 0;
#else
  RELEASE(spinlock);
#endif
}
