/* Copyright (C) 1998, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <shlib-compat.h>

struct timeval32
{
    int tv_sec, tv_usec;
};

struct timex32 {
	unsigned int modes;	/* mode selector */
	long offset;		/* time offset (usec) */
	long freq;		/* frequency offset (scaled ppm) */
	long maxerror;		/* maximum error (usec) */
	long esterror;		/* estimated error (usec) */
	int status;		/* clock command/status */
	long constant;		/* pll time constant */
	long precision;		/* clock precision (usec) (read only) */
	long tolerance;		/* clock frequency tolerance (ppm)
				 * (read only)
				 */
	struct timeval32 time;	/* (read only) */
	long tick;		/* (modified) usecs between clock ticks */

	long ppsfreq;           /* pps frequency (scaled ppm) (ro) */
	long jitter;            /* pps jitter (us) (ro) */
	int shift;              /* interval duration (s) (shift) (ro) */
	long stabil;            /* pps stability (scaled ppm) (ro) */
	long jitcnt;            /* jitter limit exceeded (ro) */
	long calcnt;            /* calibration intervals (ro) */
	long errcnt;            /* calibration errors (ro) */
	long stbcnt;            /* stability limit exceeded (ro) */

	int  :32; int  :32; int  :32; int  :32;
	int  :32; int  :32; int  :32; int  :32;
	int  :32; int  :32; int  :32; int  :32;
};

#define TIMEVAL		timeval32
#define TIMEX		timex32
#define ADJTIME		__adjtime_tv32
#define ADJTIMEX(x)	__adjtimex_tv32 (x)
#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_1)
#define LINKAGE
#else
#define LINKAGE		static
#endif

LINKAGE int ADJTIME (const struct TIMEVAL *itv, struct TIMEVAL *otv);
extern int ADJTIMEX (struct TIMEX *);

#include <sysdeps/unix/sysv/linux/adjtime.c>

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_1)
compat_symbol (libc, __adjtime_tv32, adjtime, GLIBC_2_0);
#endif

#undef TIMEVAL
#define TIMEVAL		timeval
#undef TIMEX
#define TIMEX		timex
#undef ADJTIME
#define ADJTIME		__adjtime_tv64
#undef ADJTIMEX
#define ADJTIMEX(x)	__syscall_adjtimex_tv64 (x)
#undef LINKAGE
#define LINKAGE		static

LINKAGE int ADJTIME (const struct TIMEVAL *itv, struct TIMEVAL *otv);
extern int ADJTIMEX (struct TIMEX *);

#include <sysdeps/unix/sysv/linux/adjtime.c>
static int missing_adjtimex = 0;

int
__adjtime (itv, otv)
     const struct timeval *itv;
     struct timeval *otv;
{
  int ret;

  if (!missing_adjtimex)
    {
      ret = __adjtime_tv64 (itv, otv);
      if (ret && errno == ENOSYS)
	missing_adjtimex = 1;
    }

  if (missing_adjtimex)
    {
      struct timeval32 itv32, otv32;

      itv32.tv_sec = itv->tv_sec;
      itv32.tv_usec = itv->tv_usec;
      ret = __adjtime_tv32 (&itv32, &otv32);
      if (ret == 0)
	{
	  otv->tv_sec = otv32.tv_sec;
	  otv->tv_usec = otv32.tv_usec;
	}
    }

  return ret;
}

versioned_symbol (libc, __adjtime, adjtime, GLIBC_2_1);

extern int __syscall_adjtimex_tv64 (struct timex *tx);

int
__adjtimex_tv64 (struct timex *tx)
{
  int ret;

  if (!missing_adjtimex)
   {
     ret = __syscall_adjtimex_tv64 (tx);
     if (ret && errno == ENOSYS)
	missing_adjtimex = 1;
   }

  if (missing_adjtimex)
    {
      struct timex32 tx32;

      tx32.modes = tx->modes;
      tx32.offset = tx->offset;
      tx32.freq = tx->freq;
      tx32.maxerror = tx->maxerror;
      tx32.esterror = tx->esterror;
      tx32.status = tx->status;
      tx32.constant = tx->constant;
      tx32.precision = tx->precision;
      tx32.tolerance = tx->tolerance;
      tx32.time.tv_sec = tx->time.tv_sec;
      tx32.time.tv_sec = tx->time.tv_usec;
      tx32.tick = tx->tick;
      tx32.ppsfreq = tx->ppsfreq;
      tx32.jitter = tx->jitter;
      tx32.shift = tx->shift;
      tx32.stabil = tx->stabil;
      tx32.jitcnt = tx->jitcnt;
      tx32.calcnt = tx->calcnt;
      tx32.errcnt = tx->errcnt;
      tx32.stbcnt = tx->stbcnt;

      ret = __adjtimex_tv32 (&tx32);
      if (ret == 0)
	{
	  tx->modes = tx32.modes;
	  tx->offset = tx32.offset;
	  tx->freq = tx32.freq;
	  tx->maxerror = tx32.maxerror;
	  tx->esterror = tx32.esterror;
	  tx->status = tx32.status;
	  tx->constant = tx32.constant;
	  tx->precision = tx32.precision;
	  tx->tolerance = tx32.tolerance;
	  tx->time.tv_sec = tx32.time.tv_sec;
	  tx->time.tv_usec = tx32.time.tv_sec;
	  tx->tick = tx32.tick;
	  tx->ppsfreq = tx32.ppsfreq;
	  tx->jitter = tx32.jitter;
	  tx->shift = tx32.shift;
	  tx->stabil = tx32.stabil;
	  tx->jitcnt = tx32.jitcnt;
	  tx->calcnt = tx32.calcnt;
	  tx->errcnt = tx32.errcnt;
	  tx->stbcnt = tx32.stbcnt;
	}
    }

  return ret;
}

strong_alias (__adjtimex_tv64, __adjtimex_tv64p);
versioned_symbol (libc, __adjtimex_tv64, __adjtimex, GLIBC_2_1);
versioned_symbol (libc, __adjtimex_tv64p, adjtimex, GLIBC_2_1);
