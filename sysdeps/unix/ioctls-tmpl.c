/* On SVR4, this #define is necessary to make <sys/ioctl.h> define
   many of the ioctls.  */
#define BSD_COMP

#include <sys/types.h>
#include <sys/param.h>

/* On SunOS 4.1, <sys/ioctl.h> and <sys/termios.h> define some symbols
   with different values, but <sys/termios.h> defines some ioctl symbols
   not in <sys/ioctl.h>, so we need it.  Our <sys/ioctl.h> should define
   them with the values from Sun's <sys/ioctl.h>, not <sys/termios.h>.
   So we include <sys/termios.h> and let <sys/ioctl.h> redefine things.
   This produces some spurious warnings.  */

#ifdef HAVE_sys_termios_h
#include <sys/termios.h>
#endif

/* This causes <sys/ioctl.h> to define some necessary data structure.  */
#ifdef sony_news
#define KANJI
#endif

#include <sys/ioctl.h>

#ifdef	SIOCGIFCONF
#include <sys/socket.h>
#include <sys/time.h>
#include <net/if.h>
#include <net/route.h>
#if	defined(SIOCGARP) && !defined(ARPOP_REQUEST)
#include <net/if_arp.h>
#endif
#ifdef	SIOCGNIT
#ifdef	HAVE_net_nit_h
#include <net/nit.h>
#else	/* No net/nit.h.  */
#undef SIOCGNIT
#undef SIOCSNIT
#endif	/* net/nit.h.  */
#endif	/* SIOCGNIT.  */
#endif	/* SIOCGIFCONF.  */

/* These exist on Sequents.  */
#ifdef SMIOSTATS
#include <sec/sec.h>
#include <sec/sm.h>
#endif
#ifdef SMIOGETREBOOT0
#include <i386/cfg.h>
#endif
#ifdef ZIOCBCMD
#include <zdc/zdc.h>
#endif

/* These exist under Ultrix, but I figured there may be others.  */
#ifdef DIOCGETPT
#include <ufs/fs.h>		/* for DIOC* */
#endif
#ifdef DEVGETGEOM
#include <sys/devio.h>
#endif

#ifdef ultrix
/* Ultrix has a conditional include that brings these in; we have to force
   their inclusion when we actually compile them.  */
#undef TCGETA
#undef TCSETA
#undef TCSETAW
#undef TCSETAF
#undef TCGETP
#undef TCSANOW
#undef TCSADRAIN
#undef TCSAFLUSH
#ifdef ELSETPID
#include <sys/un.h> /* get sockaddr_un for elcsd.h */
#include <elcsd.h>
#endif
#ifdef DKIOCDOP
#include <sys/dkio.h>
#endif
/* Couldn't find the header where the structures used by these are
   defined; it looks like an unbundled LAT package or something.  */
#undef LIOCSOL
#undef LIOCRES
#undef LIOCCMD
#undef LIOCINI
#undef LIOCTTYI
#undef LIOCCONN
/* struct mtop hasn't been in sys/mtio.h since 4.1 */
#undef MTIOCTOP
#undef MTIOCGET
#endif

#if defined(__osf__) && defined(__alpha__)
#include <sys/ioctl_compat.h>	/* To get TIOCGETP, etc.  */
#include <alpha/pt.h>		/* for DIOC* */
#include <sys/mtio.h>		/* for MTIOC* */
/* The binlog_getstatus structure doesn't seem to be defined.  */
#undef BINLOG_GETSTATUS
/* Can't find `struct ifdata' anywhere.  */
#undef SIOCMANREQ
#undef SIOCGETEVENTS
/* OSF/1 smells an awful lot like Ultrix.  */
#undef TCGETA
#undef TCSETA
#undef TCSETAF
#undef TCSETAW
/* This macro looks screwed in sys/devio.h.  */
#undef DEV_DISKPART
/* This is in sys/dkio.h, but we don't need it.  */
#undef DKIOCACC
#undef DKIOCDOP
#undef DKIOCEXCL
#undef DKIOCGET
#undef DKIOCHDR
/* Introduced by OSF/1 2.0.  */
#undef FIOPIPESTAT
#undef SIOCSRREQR
#undef SIOCSRREQW
#undef SRVC_REQUEST
#endif

#define	DEFINE(name, value) \
  printf("#define %s 0x%.8x\n", (name), (value))

int
main()
{
  REQUESTS

  exit(0);
  return 0;
}
