#ifndef _SYS_PARAM_H
#define _SYS_PARAM_H

#include <limits.h>
#include <linux/limits.h>
#include <linux/param.h>

#include <sys/types.h>

/* Don't change it. H.J. */
#ifdef OLD_LINUX
#undef	MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 	8	/* max length of hostname */
#endif

#ifndef howmany
#define howmany(x, y)	(((x)+((y)-1))/(y))
#endif

#ifndef roundup
#define roundup(x, y)	((((x)+((y)-1))/(y))*(y))
#endif

#define MAXPATHLEN      PATH_MAX
#define NOFILE          OPEN_MAX

/*  Following the information of some of the kernel people I here assume
 *  that block size (i.e. the value of stat.st_blocks) for all filesystem
 *  is 512 bytes.  If not tell me or HJ.  -- Uli */
#define DEV_BSIZE       512

#endif
