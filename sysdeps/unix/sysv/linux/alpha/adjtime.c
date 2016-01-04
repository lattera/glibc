/* Copyright (C) 1998-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <shlib-compat.h>
#include <sysdep.h>
#include <sys/time.h>


#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_1)
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
#define ADJTIME		attribute_compat_text_section __adjtime_tv32
#define ADJTIMEX(x)	INLINE_SYSCALL (old_adjtimex, 1, x)
#define ADJTIMEX32(x)	INLINE_SYSCALL (old_adjtimex, 1, x)

#include <sysdeps/unix/sysv/linux/adjtime.c>

int attribute_compat_text_section
__adjtimex_tv32 (struct timex32 *tx) { return ADJTIMEX (tx); }

strong_alias (__adjtimex_tv32, __adjtimex_tv32_1);
strong_alias (__adjtimex_tv32, __adjtimex_tv32_2);
compat_symbol (libc, __adjtimex_tv32_1, __adjtimex, GLIBC_2_0);
compat_symbol (libc, __adjtimex_tv32_2, adjtimex, GLIBC_2_0);
compat_symbol (libc, __adjtime_tv32, adjtime, GLIBC_2_0);
#endif /* SHLIB_COMPAT */

#undef TIMEVAL
#undef TIMEX
#undef ADJTIME
#undef ADJTIMEX
#define TIMEVAL		timeval
#define TIMEX		timex
#define ADJTIMEX(x)	INLINE_SYSCALL (adjtimex, 1, x)

#include <sysdeps/unix/sysv/linux/adjtime.c>

int
__adjtimex_tv64 (struct timex *tx) { return ADJTIMEX (tx); }

libc_hidden_ver (__adjtimex_tv64, __adjtimex)
strong_alias (__adjtimex_tv64, __adjtimex_tv64p);
weak_alias (__adjtimex_tv64, ntp_adjtime);
versioned_symbol (libc, __adjtimex_tv64, __adjtimex, GLIBC_2_1);
versioned_symbol (libc, __adjtimex_tv64p, adjtimex, GLIBC_2_1);
versioned_symbol (libc, __adjtime, adjtime, GLIBC_2_1);
