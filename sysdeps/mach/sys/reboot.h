/*
 * Mach Operating System
 * Copyright (C) 1993,1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log$
 * Revision 1.4  2002/07/06 06:36:00  aj
 * 	* sysdeps/ia64/fpu/e_acos.S: Added text of Intel license.
 * 	* sysdeps/ia64/fpu/e_acosf.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_acosl.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_asin.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_asinf.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_asinl.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_atan2.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_atan2f.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_cosh.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_coshf.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_coshl.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_exp.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_expf.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_fmod.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_fmodf.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_fmodl.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_hypot.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_hypotf.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_hypotl.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_log.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_logf.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_pow.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_powf.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_powl.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_remainder.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_remainderf.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_remainderl.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_scalb.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_scalbf.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_scalbl.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_sinh.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_sinhf.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_sinhl.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_sqrt.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_sqrtf.S: Likewise.
 * 	* sysdeps/ia64/fpu/e_sqrtl.S: Likewise.
 * 	* sysdeps/ia64/fpu/libm_atan2_req.S: Likewise.
 * 	* sysdeps/ia64/fpu/libm_error.c: Likewise.
 * 	* sysdeps/ia64/fpu/libm_frexp4.S: Likewise.
 * 	* sysdeps/ia64/fpu/libm_frexp4f.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_frexpl.c: Likewise.
 * 	* sysdeps/ia64/fpu/s_ilogb.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_ilogbf.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_ilogbl.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_ldexp.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_ldexpf.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_ldexpl.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_log1p.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_log1pf.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_log1pl.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_logb.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_logbf.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_logbl.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_modf.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_modff.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_modfl.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_nearbyint.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_nearbyintf.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_nearbyintl.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_rint.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_rintf.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_rintl.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_round.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_roundf.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_roundl.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_scalbn.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_scalbnf.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_scalbnl.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_significand.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_significandf.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_significandl.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_tan.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_tanf.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_tanl.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_trunc.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_truncf.S: Likewise.
 * 	* sysdeps/ia64/fpu/s_truncl.S: Likewise.
 * 	* sysdeps/ieee754/dbl-64/doasin.c: changed copyright notice to
 * 	reflect IBM donation of math library to FSF
 * 	* sysdeps/ieee754/dbl-64/dosincos.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/e_asin.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/e_atan2.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/e_exp.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/e_log.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/e_pow.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/e_remainder.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/e_sqrt.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/halfulp.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/mpa.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/mpatan.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/mpatan2.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/mpexp.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/mplog.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/mpsqrt.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/mptan.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/s_atan.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/s_sin.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/s_tan.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/sincos32.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/slowexp.c: Likewise.
 * 	* sysdeps/ieee754/dbl-64/slowpow.c: Likewise.
 * 	* sysdeps/gnu/netinet/udp.h: Added BSD copying permission notice
 * 	* sysdeps/vax/__longjmp.c: Likewise.
 * 	* sysdeps/vax/setjmp.c: Likewise.
 * 	* libio/filedoalloc.c: Fixed BSD copying permission notice to remove
 * 	advertising clause
 * 	* sysdeps/vax/htonl.s: Likewise.
 * 	* sysdeps/vax/htons.s: Likewise.
 * 	* libio/wfiledoalloc.c: Likewise.
 * 	* stdlib/random.c: Likewise.
 * 	* stdlib/random_r.c: Likewise.
 * 	* sysdeps/mach/sys/reboot.h: Likewise.
 *         * inet/getnameinfo.c: Deleted advertising clause from Inner Net License
 *         * sysdeps/posix/getaddrinfo.c: Likewise.
 *         * sunrpc/des_impl.c: Updated license permission notice to Lesser GPL
 *           and corrected pointer to point to the correct license.
 *
 * Revision 1.3  2000/03/27 04:09:08  roland
 * 2000-03-26  Roland McGrath  <roland@baalperazim.frob.com>
 *
 * 	* sysdeps/mach/sys/reboot.h: Include <features.h>.
 * 	(reboot): Declare it.
 *
 * Revision 1.2  1998/05/29 10:19:59  drepper
 * Use __ASSEMBLER__ test macro not ASSEMBLER.
 *
 * Revision 1.1  1993/08/03 22:25:15  roland
 * entered into RCS
 *
 * Revision 2.8  93/03/11  13:46:40  danner
 * 	u_long -> u_int.
 * 	[93/03/09            danner]
 *
 * Revision 2.7  92/05/21  17:25:11  jfriedl
 * 	Appended 'U' to constants that would otherwise be signed.
 * 	[92/05/16            jfriedl]
 *
 * Revision 2.6  91/06/19  11:59:44  rvb
 * 	Second byte of boothowto is flags for "startup" program.
 * 	[91/06/18            rvb]
 * 	Add ifndef __ASSEMBLER__ so that vax_init.s can include it.
 * 	[91/06/11            rvb]
 *
 * Revision 2.5  91/05/14  17:40:11  mrt
 * 	Correcting copyright
 *
 * Revision 2.4  91/02/05  17:56:48  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:49:12  mrt]
 *
 * Revision 2.3  90/08/27  22:12:56  dbg
 * 	Added definitions used by Mach Kernel: RB_DEBUGGER, RB_UNIPROC,
 * 	RB_NOBOOTRC, RB_ALTBOOT.  Moved RB_KDB to 0x04 (Mach value).
 * 	Removed RB_RDONLY, RB_DUMP, RB_NOSYNC.
 * 	[90/08/14            dbg]
 *
 */

/*
   Copyright (C) 1982, 1986, 1988 Regents of the University of California.
   All rights reserved.
 
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   4. Neither the name of the University nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
   SUCH DAMAGE.*/

/*
 *	@(#)reboot.h	7.5 (Berkeley) 6/27/88
 */

#ifndef	_SYS_REBOOT_H_
#define	_SYS_REBOOT_H_

#include <features.h>

/*
 * Arguments to reboot system call.
 * These are converted to switches, and passed to startup program,
 * and on to init.
 */
#define	RB_AUTOBOOT	0	/* flags for system auto-booting itself */

#define	RB_ASKNAME	0x01	/* -a: ask for file name to reboot from */
#define	RB_SINGLE	0x02	/* -s: reboot to single user only */
#define	RB_KDB		0x04	/* -d: kernel debugger symbols loaded */
#define	RB_HALT		0x08	/* -h: enter KDB at bootup */
				/*     for host_reboot(): don't reboot,
				       just halt */
#define	RB_INITNAME	0x10	/* -i: name given for /etc/init (unused) */
#define	RB_DFLTROOT	0x20	/*     use compiled-in rootdev */
#define	RB_NOBOOTRC	0x20	/* -b: don't run /etc/rc.boot */
#define RB_ALTBOOT	0x40	/*     use /boot.old vs /boot */
#define	RB_UNIPROC	0x80	/* -u: start only one processor */

#define	RB_SHIFT	8	/* second byte is for ux */

#define	RB_DEBUGGER	0x1000	/*     for host_reboot(): enter kernel
				       debugger from user level */

/*
 * Constants for converting boot-style device number to type,
 * adaptor (uba, mba, etc), unit number and partition number.
 * Type (== major device number) is in the low byte
 * for backward compatibility.  Except for that of the "magic
 * number", each mask applies to the shifted value.
 * Format:
 *	 (4) (4) (4) (4)  (8)     (8)
 *	--------------------------------
 *	|MA | AD| CT| UN| PART  | TYPE |
 *	--------------------------------
 */
#define	B_ADAPTORSHIFT		24
#define	B_ADAPTORMASK		0x0f
#define	B_ADAPTOR(val)		(((val) >> B_ADAPTORSHIFT) & B_ADAPTORMASK)
#define B_CONTROLLERSHIFT	20
#define B_CONTROLLERMASK	0xf
#define	B_CONTROLLER(val)	(((val)>>B_CONTROLLERSHIFT) & B_CONTROLLERMASK)
#define B_UNITSHIFT		16
#define B_UNITMASK		0xf
#define	B_UNIT(val)		(((val) >> B_UNITSHIFT) & B_UNITMASK)
#define B_PARTITIONSHIFT	8
#define B_PARTITIONMASK		0xff
#define	B_PARTITION(val)	(((val) >> B_PARTITIONSHIFT) & B_PARTITIONMASK)
#define	B_TYPESHIFT		0
#define	B_TYPEMASK		0xff
#define	B_TYPE(val)		(((val) >> B_TYPESHIFT) & B_TYPEMASK)

#define	B_MAGICMASK	((u_int)0xf0000000U)
#define	B_DEVMAGIC	((u_int)0xa0000000U)

#define MAKEBOOTDEV(type, adaptor, controller, unit, partition) \
	(((type) << B_TYPESHIFT) | ((adaptor) << B_ADAPTORSHIFT) | \
	((controller) << B_CONTROLLERSHIFT) | ((unit) << B_UNITSHIFT) | \
	((partition) << B_PARTITIONSHIFT) | B_DEVMAGIC)


#ifdef	KERNEL
#ifndef	__ASSEMBLER__
extern int boothowto;
#endif	/* __ASSEMBLER__ */
#endif

__BEGIN_DECLS

/* Reboot or halt the system.  */
extern int reboot (int __howto) __THROW;

__END_DECLS


#endif	/* _SYS_REBOOT_H_ */
