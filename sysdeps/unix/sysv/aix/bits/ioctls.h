/* Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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

#ifndef _SYS_IOCTL_H
# error "Never use <bits/ioctls.h> directly; include <sys/ioctl.h> instead."
#endif


#define	IOCPARM_MASK	0x7f		/* parameters must be < 128 bytes */
#define	IOC_VOID	0x20000000	/* no parameters */
#define	IOC_OUT		0x40000000	/* copy out parameters */
#define	IOC_IN		(0x40000000<<1)	/* copy in parameters */
#define	IOC_INOUT	(IOC_IN|IOC_OUT)
#define	_IO(x,y)	(IOC_VOID|(x<<8)|y)
#define	_IOR(x,y,t)	(IOC_OUT|((sizeof(t)&IOCPARM_MASK)<<16)|(x<<8)|y)
#define	_IOW(x,y,t)	(IOC_IN|((sizeof(t)&IOCPARM_MASK)<<16)|(x<<8)|y)
#define	_IOWR(x,y,t)	(IOC_INOUT|((sizeof(t)&IOCPARM_MASK)<<16)|(x<<8)|y)

#define	TIOCGETD	_IOR('t', 0, int)	/* get line discipline */
#define	TIOCSETD	_IOW('t', 1, int)	/* set line discipline */
#define	TIOCHPCL	_IO('t', 2)		/* hang up on last close */
#define	TIOCMODG	_IOR('t', 3, int)	/* get modem control state */
#define	TIOCMODS	_IOW('t', 4, int)	/* set modem control state */
#define	TIOCGETP	_IOR('t', 8,struct sgttyb)/* get parameters -- gtty */
#define	TIOCSETP	_IOW('t', 9,struct sgttyb)/* set parameters -- stty */
#define	TIOCSETN	_IOW('t',10,struct sgttyb)/* as above, but no flushtty */
#define	TIOCEXCL	_IO('t', 13)		/* set exclusive use of tty */
#define	TIOCNXCL	_IO('t', 14)		/* reset exclusive use of tty */
#define	TIOCFLUSH	_IOW('t', 16, int)	/* flush buffers */
#define	TIOCSETC	_IOW('t',17,struct tchars)/* set special characters */
#define	TIOCGETC	_IOR('t',18,struct tchars)/* get special characters */
#define		TANDEM		0x00000001	/* send stopc on out q full */
#define		CBREAK		0x00000002	/* half-cooked mode */
#define		LCASE		0x00000004	/* simulate lower case */
#define		CRMOD		0x00000010	/* map \r to \r\n on output */
#define		RAW		0x00000020	/* no i/o processing */
#define		ODDP		0x00000040	/* get/send odd parity */
#define		EVENP		0x00000080	/* get/send even parity */
#define		ANYP		0x000000c0	/* get any parity/send none */
#define		CRDELAY		0x00000300	/* \r delay */
#define		TBDELAY		0x00000c00	/* horizontal tab delay */
#define		XTABS		0x00000c00	/* expand tabs on output */
#define		BSDELAY		0x00001000	/* \b delay */
#define		VTDELAY		0x00002000	/* vertical tab delay */
#define		NLDELAY		0x0000c000	/* \n delay */
#define			NL2	0x00008000	/* vt05 */
#define			NL3	0x0000c000
#define		ALLDELAY	(NLDELAY|TBDELAY|CRDELAY|VTDELAY|BSDELAY)
#define		PRTERA		0x00020000	/* \ ... / erase */
#define		CRTERA		0x00040000	/* " \b " to wipe out char */
#define		TILDE		0x00080000	/* hazeltine tilde kludge */
#define		LITOUT		0x00200000	/* literal output */
#define		CRTBS		0x00400000	/* do backspacing for crt */
#define		MDMBUF		0x00800000	/* dtr pacing */
#define		NOHANG		0x01000000	/* no SIGHUP on carrier drop */
#define		L001000		0x02000000
#define		CRTKIL		0x04000000	/* kill line with " \b " */
#define		PASS8		0x08000000
#define		CTLECH		0x10000000	/* echo control chars as ^X */
#define		DECCTQ		0x40000000	/* only ^Q starts after ^S */
#define		NOFLUSH		0x80000000	/* no output flush on signal */


/* SYS V REL. 4 PTY IOCTLs    */
#define UNLKPT          _IO('t',70)             /* unlock slave pty */
#define ISPTM           _IO('t',71)             /* ret. maj+min of pty master */
#define ISPTS           _IO('t',73)             /* return maj+min of slave */
#define GRTPT           _IO('t',74)             /* grantpt slave pty*/
#define RLOGIND         _IO('t',75)             /* for rlogind protocol in ptydd */
#define TELNETDP        _IO('t',76)             /* for telnetd protocol in ptydd */

#define	TIOCCONS	_IOW('t', 98, int)	/* become virtual console */
#define	TIOCGSID	_IOR('t', 72, int)	/* get the tty session id */

						/* locals, from 127 down */
#define	TIOCLBIS	_IOW('t', 127, int)	/* bis local mode bits */
#define	TIOCLBIC	_IOW('t', 126, int)	/* bic local mode bits */
#define	TIOCLSET	_IOW('t', 125, int)	/* set entire mode word */
#define	TIOCLGET	_IOR('t', 124, int)	/* get local modes */
#define		LCRTBS		(CRTBS>>16)
#define		LPRTERA		(PRTERA>>16)
#define		LCRTERA		(CRTERA>>16)
#define		LTILDE		(TILDE>>16)
#define		LMDMBUF		(MDMBUF>>16)
#define		LLITOUT		(LITOUT>>16)
#define		LTOSTOP		(TOSTOP>>16)
#define		LFLUSHO		(FLUSHO>>16)
#define		LNOHANG		(NOHANG>>16)
#define		LCRTKIL		(CRTKIL>>16)
#define		LPASS8		(PASS8>>16)
#define		LCTLECH		(CTLECH>>16)
#define		LPENDIN		(PENDIN>>16)
#define		LDECCTQ		(DECCTQ>>16)
#define		LNOFLSH		(NOFLUSH>>16)
#define	TIOCSBRK	_IO('t', 123)		/* set break bit */
#define	TIOCCBRK	_IO('t', 122)		/* clear break bit */
#define	TIOCSDTR	_IO('t', 121)		/* set data terminal ready */
#define	TIOCCDTR	_IO('t', 120)		/* clear data terminal ready */
#define	TIOCGPGRP	_IOR('t', 119, int)	/* get process group */
#define	TIOCSPGRP	_IOW('t', 118, int)	 /* set process gorup */
#define	TIOCSLTC	_IOW('t',117,struct ltchars)/* set local special chars */
#define	TIOCGLTC	_IOR('t',116,struct ltchars)/* get local special chars */
#define	TIOCOUTQ	_IOR('t', 115, int)	/* output queue size */
#define	TIOCSTI		_IOW('t', 114, char)	/* simulate terminal input */
#define	TIOCNOTTY	_IO('t', 113)		/* void tty association */
#define	TIOCPKT		_IOW('t', 112, int)	/* pty: set/clear packet mode */
#define		TIOCPKT_DATA		0x00	/* data packet */
#define		TIOCPKT_FLUSHREAD	0x01	/* flush packet */
#define		TIOCPKT_FLUSHWRITE	0x02	/* flush packet */
#define		TIOCPKT_STOP		0x04	/* stop output */
#define		TIOCPKT_START		0x08	/* start output */
#define		TIOCPKT_NOSTOP		0x10	/* no more ^S, ^Q */
#define		TIOCPKT_DOSTOP		0x20	/* now do ^S ^Q */
#define	TIOCSTOP	_IO('t', 111)		/* stop output, like ^S */
#define	TIOCSTART	_IO('t', 110)		/* start output, like ^Q */
#define	TIOCMSET	_IOW('t', 109, int)	/* set all modem bits */
#define	TIOCMBIS	_IOW('t', 108, int)	/* bis modem bits */
#define	TIOCMBIC	_IOW('t', 107, int)	/* bic modem bits */
#define	TIOCMGET	_IOR('t', 106, int)	/* get all modem bits */
#define	TIOCREMOTE	_IOW('t', 105, int)	/* remote input editing */
#define	TIOCGWINSZ	_IOR('t', 104, struct winsize) 	/* get window size */
#define	TIOCSWINSZ	_IOW('t', 103, struct winsize) 	/* set window size */
#define	TIOCUCNTL	_IOW('t', 102, int)	/* pty: set/clr usr cntl mode */
/* SLIP (Serial Line IP) ioctl's */
#define	SLIOCGUNIT	_IOR('t', 101, int)	/* get slip unit number */
#define SLIOCSFLAGS     _IOW('t', 89, int)      /* set configuration flags */
#define SLIOCGFLAGS     _IOR('t', 90, int)      /* get configuration flags */
#define SLIOCSATTACH    _IOWR('t', 91, int)	/* Attach slip i.f. to tty  */
#define		UIOCCMD(n)	_IO('u', n)		/* usr cntl op "n" */

#define	OTTYDISC	0		/* old, v7 std tty driver */
#define	NETLDISC	1		/* line discip for berk net */
#define	NTTYDISC	2		/* new tty discipline */
#define	TABLDISC	3		/* tablet discipline */
#define	SLIPDISC	4		/* serial IP discipline */

#define	FIOCLEX		_IO('f', 1)		/* set close on exec    */
#define	FIONCLEX	_IO('f', 2)		/* clear close on exec  */
/* another local */

#define	FIONREAD	_IOR('f', 127, int)	/* get # bytes to read */
#define	FIONBIO		_IOW('f', 126, int)	/* set/clear non-blocking i/o */
#define	FIOASYNC	_IOW('f', 125, int)	/* set/clear async i/o */

#define	FIOSETOWN	_IOW('f', 124, int)	/* set owner */
#define	FIOGETOWN	_IOR('f', 123, int)	/* get owner */
#define	FIOASYNCQX	_IOW('f', 122, int)	/* set/clear async queueing */

/* socket i/o controls */
#define	SIOCSHIWAT	_IOW('s',  0, int)		/* set high watermark */
#define	SIOCGHIWAT	_IOR('s',  1, int)		/* get high watermark */
#define	SIOCSLOWAT	_IOW('s',  2, int)		/* set low watermark */
#define	SIOCGLOWAT	_IOR('s',  3, int)		/* get low watermark */
#define	SIOCATMARK	_IOR('s',  7, int)		/* at oob mark? */
#define	SIOCSPGRP	_IOW('s',  8, int)		/* set process group */
#define	SIOCGPGRP	_IOR('s',  9, int)		/* get process group */

#define	SIOCADDRT	(int)_IOW('r', 10, struct ortentry)	/* add route */
#define	SIOCDELRT	(int)_IOW('r', 11, struct ortentry)	/* delete route */

#define	SIOCSIFADDR	(int)_IOW('i', 12, struct oifreq)	/* set ifnet address */
#define	OSIOCGIFADDR	(int)_IOWR('i',13, struct oifreq)	/* get ifnet address */
#define	SIOCGIFADDR	(int)_IOWR('i',33, struct oifreq)	/* get ifnet address */
#define	SIOCSIFDSTADDR	(int)_IOW('i', 14, struct oifreq)	/* set p-p address */
#define	OSIOCGIFDSTADDR	(int)_IOWR('i',15, struct oifreq)	/* get p-p address */
#define	SIOCGIFDSTADDR	(int)_IOWR('i',34, struct oifreq)	/* get p-p address */
#define	SIOCSIFFLAGS	(int)_IOW('i', 16, struct oifreq)	/* set ifnet flags */
#define	SIOCGIFFLAGS	(int)_IOWR('i',17, struct oifreq)	/* get ifnet flags */
#define	OSIOCGIFBRDADDR	(int)_IOWR('i',18, struct oifreq)	/* get broadcast addr */
#define	SIOCGIFBRDADDR	(int)_IOWR('i',35, struct oifreq)	/* get broadcast addr */
#define	SIOCSIFBRDADDR	(int)_IOW('i',19, struct oifreq)	/* set broadcast addr */
#define	OSIOCGIFCONF	(int)_IOWR('i',20, struct ifconf)	/* get ifnet list */
#define	CSIOCGIFCONF	(int)_IOWR('i',36, struct ifconf)	/* get ifnet list */
#define	SIOCGIFCONF	(int)_IOWR('i',69, struct ifconf)	/* get ifnet list */
#define	OSIOCGIFNETMASK	(int)_IOWR('i',21, struct oifreq)	/* get net addr mask */
#define	SIOCGIFNETMASK	(int)_IOWR('i',37, struct oifreq)	/* get net addr mask */
#define	SIOCSIFNETMASK	(int)_IOW('i',22, struct oifreq)	/* set net addr mask */
#define	SIOCGIFMETRIC	(int)_IOWR('i',23, struct oifreq)	/* get IF metric */
#define	SIOCSIFMETRIC	(int)_IOW('i',24, struct oifreq)	/* set IF metric */
#define	SIOCDIFADDR	(int)_IOW('i',25, struct oifreq)	/* delete IF addr */
#define	SIOCAIFADDR	(int)_IOW('i',26, struct ifaliasreq)	/* add/chg IF alias */
#define	SIOCSIFSUBCHAN	(int)_IOW('i',27, struct oifreq)	/* set subchannel adr.*/
#define SIOCSIFNETDUMP  (int)_IOW('i',28, struct oifreq)        /* set netdump fastwrt*/

#define	SIOCSARP	(int)_IOW('i', 30, struct arpreq)	/* set arp entry */
#define	OSIOCGARP	(int)_IOWR('i',31, struct arpreq)	/* get arp entry */
#define	SIOCGARP	(int)_IOWR('i',38, struct arpreq)	/* get arp entry */
#define	SIOCDARP	(int)_IOW('i', 32, struct arpreq)	/* delete arp entry */

#define	SIOCSIFOPTIONS	(int)_IOW('i', 41, struct oifreq)	/* set ifnet options */
#define	SIOCGIFOPTIONS	(int)_IOWR('i',42, struct oifreq)	/* get ifnet options */
#define	SIOCADDMULTI	(int)_IOW('i', 49, struct ifreq)	/* add multicast addr */
#define	SIOCDELMULTI	(int)_IOW('i', 50, struct ifreq)	/* del multicast addr */
#define	SIOCGETVIFCNT	(int)_IOWR('u', 51, struct sioc_vif_req)/* vif pkt cnt */
#define	SIOCGETSGCNT	(int)_IOWR('u', 52, struct sioc_sg_req) /* s,g pkt cnt */

#define	SIOCADDNETID	(int)_IOW('i',87, struct oifreq)	/* set netids */
#define	SIOCSIFMTU	(int)_IOW('i',88, struct oifreq)	/* set mtu */
#define	SIOCGIFMTU	(int)_IOWR('i',86, struct oifreq)	/* get mtu */

#define SIOCSNETOPT     (int)_IOW('i', 90, struct optreq) /* set network option */
#define SIOCGNETOPT     (int)_IOWR('i', 91, struct optreq) /* get network option */
#define SIOCDNETOPT     (int)_IOWR('i', 92, struct optreq) /* set default */

#define	SIOCSX25XLATE	(int)_IOW('i', 99, struct oifreq)	/* set xlate tab */
#define	SIOCGX25XLATE	(int)_IOWR('i',100, struct oifreq)	/* get xlate tab */
#define	SIOCDX25XLATE	(int)_IOW('i', 101, struct oifreq)	/* delete xlate tab */

#define SIOCIFDETACH	(int)_IOW('i', 102, struct ifreq)	/* detach an ifnet */
#define SIOCIFATTACH	(int)_IOW('i', 103, struct ifreq)	/* attach an ifnet */

#define	SIOCGNMTUS	(int)_IO('i',110) /* get NMTUs */
#define	SIOCGETMTUS	(int)_IO('i',111) /* get common_mtus */
#define	SIOCADDMTU	(int)_IOW('i',112, int) /* add mtu  */
#define	SIOCDELMTU	(int)_IOW('i',113, int) /* delete mtu */

#define SIOCGIFGIDLIST  (int)_IO('i', 104)                   /* get gidlist */
#define SIOCSIFGIDLIST  (int)_IO('i', 105)                   /* set gidlist */

#define SIOCGSIZIFCONF  (int)_IOR('i', 106, int) /* get size for SIOCGIFCONF */

#define SIOCIF_ATM_UBR      	(int)_IOW('i',120,struct ifreq)  /* set ubr rate */
#define SIOCIF_ATM_SNMPARP      (int)_IOW('i',121,struct ifreq)  /* atm snmp arp */
#define SIOCIF_ATM_IDLE         (int)_IOW('i',122,struct ifreq)  /* set idle time */
#define SIOCIF_ATM_DUMPARP      (int)_IOW('i',123,struct ifreq)  /* atm dump arp */
#define SIOCIF_ATM_SVC		(int)_IOW('i',124,struct ifreq)  /* atmif init */
#define SIOCIF_ATM_DARP		(int)_IOW('i',125,struct ifreq)  /* del atmarp */
#define SIOCIF_ATM_GARP		(int)_IOW('i',126,struct ifreq)  /* get atmarp */
#define SIOCIF_ATM_SARP		(int)_IOW('i',127,struct ifreq)  /* set atmarp */

#define	SIOCGISNO	(int)_IOWR('i',107, struct oifreq)	/* get IF network options */
#define	SIOCSISNO 	(int)_IOW('i', 108, struct oifreq)	/* set IF network options */
#define SIOCGIFBAUDRATE (int)_IOWR('i', 109, struct oifreq)     /* get ifnet's if_baudrate */
