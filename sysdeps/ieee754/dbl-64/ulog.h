/*
 * IBM Accurate Mathematical Library
 * Written by International Business Machines Corp.
 * Copyright (C) 2001-2018 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/******************************************************************/
/*                                                                */
/* MODULE_NAME:ulog.h                                             */
/*                                                                */
/* common data and variables prototype and definition             */
/******************************************************************/

#ifndef ULOG_H
#define ULOG_H

#ifdef BIG_ENDI
  static const number
  /* polynomial I */
/**/ a2             = {{0xbfe00000, 0x0001aa8f} }, /* -0.500... */
/**/ a3             = {{0x3fd55555, 0x55588d2e} }, /*  0.333... */
  /* polynomial II */
/**/ b0             = {{0x3fd55555, 0x55555555} }, /*  0.333... */
/**/ b1             = {{0xbfcfffff, 0xffffffbb} }, /* -0.249... */
/**/ b2             = {{0x3fc99999, 0x9999992f} }, /*  0.199... */
/**/ b3             = {{0xbfc55555, 0x556503fd} }, /* -0.166... */
/**/ b4             = {{0x3fc24924, 0x925b3d62} }, /*  0.142... */
/**/ b5             = {{0xbfbffffe, 0x160472fc} }, /* -0.124... */
/**/ b6             = {{0x3fbc71c5, 0x25db58ac} }, /*  0.111... */
/**/ b7             = {{0xbfb9a4ac, 0x11a2a61c} }, /* -0.100... */
/**/ b8             = {{0x3fb75077, 0x0df2b591} }, /*  0.091... */
  /* constants    */
/**/ sqrt_2         = {{0x3ff6a09e, 0x667f3bcc} }, /* sqrt(2)   */
/**/ h1             = {{0x3fd2e000, 0x00000000} }, /* 151/2**9  */
/**/ h2             = {{0x3f669000, 0x00000000} }, /* 361/2**17 */
/**/ delu           = {{0x3f700000, 0x00000000} }, /* 1/2**8    */
/**/ delv           = {{0x3ef00000, 0x00000000} }, /* 1/2**16   */
/**/ ln2a           = {{0x3fe62e42, 0xfefa3800} }, /* ln(2) 43 bits */
/**/ ln2b           = {{0x3d2ef357, 0x93c76730} }, /* ln(2)-ln2a    */
/**/ two54          = {{0x43500000, 0x00000000} }, /* 2**54         */
/**/ u03            = {{0x3f9eb851, 0xeb851eb8} }; /* 0.03          */

#else
#ifdef LITTLE_ENDI
  static const number
  /* polynomial I */
/**/ a2             = {{0x0001aa8f, 0xbfe00000} }, /* -0.500... */
/**/ a3             = {{0x55588d2e, 0x3fd55555} }, /*  0.333... */
  /* polynomial II */
/**/ b0             = {{0x55555555, 0x3fd55555} }, /*  0.333... */
/**/ b1             = {{0xffffffbb, 0xbfcfffff} }, /* -0.249... */
/**/ b2             = {{0x9999992f, 0x3fc99999} }, /*  0.199... */
/**/ b3             = {{0x556503fd, 0xbfc55555} }, /* -0.166... */
/**/ b4             = {{0x925b3d62, 0x3fc24924} }, /*  0.142... */
/**/ b5             = {{0x160472fc, 0xbfbffffe} }, /* -0.124... */
/**/ b6             = {{0x25db58ac, 0x3fbc71c5} }, /*  0.111... */
/**/ b7             = {{0x11a2a61c, 0xbfb9a4ac} }, /* -0.100... */
/**/ b8             = {{0x0df2b591, 0x3fb75077} }, /*  0.091... */
  /* constants    */
/**/ sqrt_2         = {{0x667f3bcc, 0x3ff6a09e} }, /* sqrt(2)   */
/**/ h1             = {{0x00000000, 0x3fd2e000} }, /* 151/2**9  */
/**/ h2             = {{0x00000000, 0x3f669000} }, /* 361/2**17 */
/**/ delu           = {{0x00000000, 0x3f700000} }, /* 1/2**8    */
/**/ delv           = {{0x00000000, 0x3ef00000} }, /* 1/2**16   */
/**/ ln2a           = {{0xfefa3800, 0x3fe62e42} }, /* ln(2) 43 bits */
/**/ ln2b           = {{0x93c76730, 0x3d2ef357} }, /* ln(2)-ln2a    */
/**/ two54          = {{0x00000000, 0x43500000} }, /* 2**54         */
/**/ u03            = {{0xeb851eb8, 0x3f9eb851} }; /* 0.03          */

#endif
#endif

#define  SQRT_2    sqrt_2.d
#define  DEL_U     delu.d
#define  DEL_V     delv.d
#define  LN2A      ln2a.d
#define  LN2B      ln2b.d
#define  U03       u03.d

#endif
