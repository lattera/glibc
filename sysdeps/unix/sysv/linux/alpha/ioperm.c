/* Copyright (C) 1992, 1996-1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger.

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

/* I/O access is restricted to ISA port space (ports 0..65535).
   Modern devices hopefully are sane enough not to put any performance
   critical registers in i/o space.

   On the first call to ioperm() or _sethae(), the entire (E)ISA port
   space is mapped into the virtual address space at address io.base.
   mprotect() calls are then used to enable/disable access to ports.  Per
   page, there are PAGE_SIZE>>IO_SHIFT I/O ports (e.g., 256 ports on a
   Low Cost Alpha based system using 8KB pages).

   Keep in mind that this code should be able to run in a 32bit address
   space.  It is therefore unreasonable to expect mmap'ing the entire
   sparse address space would work (e.g., the Low Cost Alpha chip has an
   I/O address space that's 512MB large!).  */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/mman.h>

#include <asm/page.h>
#include <asm/system.h>

#define PATH_ALPHA_SYSTYPE	"/etc/alpha_systype"
#define PATH_CPUINFO		"/proc/cpuinfo"

#define MAX_PORT	0x10000
#define vuip		volatile unsigned int *

#define JENSEN_IO_BASE		(0xfffffc0300000000UL)
#define JENSEN_SPARSE_MEM	(0xfffffc0200000000UL)

/* With respect to the I/O architecture, APECS and LCA are identical,
   so the following defines apply to LCA as well.  */
#define APECS_IO_BASE		(0xfffffc01c0000000UL)
#define APECS_SPARSE_MEM	(0xfffffc0200000000UL)
#define APECS_DENSE_MEM		(0xfffffc0300000000UL)

/* The same holds for CIA and PYXIS.  */
#define CIA_IO_BASE		(0xfffffc8580000000UL)
#define CIA_SPARSE_MEM		(0xfffffc8000000000UL)
#define CIA_DENSE_MEM		(0xfffffc8600000000UL)

/* SABLE is EV4, GAMMA is EV5 */
#define T2_IO_BASE		(0xfffffc03a0000000UL)
#define T2_SPARSE_MEM		(0xfffffc0200000000UL)
#define T2_DENSE_MEM		(0xfffffc03c0000000UL)

#define GAMMA_IO_BASE		(0xfffffc83a0000000UL)
#define GAMMA_SPARSE_MEM	(0xfffffc8200000000UL)
#define GAMMA_DENSE_MEM		(0xfffffc83c0000000UL)

/* these are for the RAWHIDE family */
#define MCPCIA_IO_BASE		(0xfffffcf980000000UL)
#define MCPCIA_SPARSE_MEM	(0xfffffcf800000000UL)
#define MCPCIA_DENSE_MEM	(0xfffffcf900000000UL)

/* Tsunami has no SPARSE space */
/* NOTE: these are hardwired to PCI bus 0 addresses!!! */
/* Also, these are PHYSICAL, as/so there's no KSEG translation */
#define TSUNAMI_IO_BASE		(0x00000801fc000000UL + 0xfffffc0000000000UL)
#define TSUNAMI_DENSE_MEM	(0x0000080000000000UL + 0xfffffc0000000000UL)

/* Polaris has SPARSE space, but we prefer to use only DENSE */
/* because of some idiosyncracies in actually using SPARSE */
#define POLARIS_IO_BASE		(0xfffffcf9fc000000UL)
#define POLARIS_DENSE_MEM	(0xfffffcf900000000UL)

typedef enum {
  IOSYS_UNKNOWN, IOSYS_JENSEN, IOSYS_APECS, IOSYS_CIA, IOSYS_T2,
  IOSYS_TSUNAMI, IOSYS_MCPCIA, IOSYS_GAMMA, IOSYS_POLARIS,
  IOSYS_CPUDEP, IOSYS_PCIDEP
} iosys_t;

typedef enum {
  IOSWIZZLE_JENSEN, IOSWIZZLE_SPARSE, IOSWIZZLE_DENSE
} ioswizzle_t;

static struct io_system {
  int		    hae_shift;
  unsigned long	int bus_memory_base;
  unsigned long	int sparse_bus_mem_base;
  unsigned long	int bus_io_base;
} io_system[] = { /* NOTE! must match iosys_t enumeration */
/* UNKNOWN */	{0, 0, 0, 0},
/* JENSEN */	{7, 0, JENSEN_SPARSE_MEM, JENSEN_IO_BASE},
/* APECS */	{5, APECS_DENSE_MEM, APECS_SPARSE_MEM, APECS_IO_BASE},
/* CIA */	{5, CIA_DENSE_MEM, CIA_SPARSE_MEM, CIA_IO_BASE},
/* T2 */	{5, T2_DENSE_MEM, T2_SPARSE_MEM, T2_IO_BASE},
/* TSUNAMI */	{0, TSUNAMI_DENSE_MEM, 0, TSUNAMI_IO_BASE},
/* MCPCIA */	{5, MCPCIA_DENSE_MEM, MCPCIA_SPARSE_MEM, MCPCIA_IO_BASE},
/* GAMMA */	{5, GAMMA_DENSE_MEM, GAMMA_SPARSE_MEM, GAMMA_IO_BASE},
/* POLARIS */	{0, POLARIS_DENSE_MEM, 0, POLARIS_IO_BASE},
/* CPUDEP */	{0, 0, 0, 0}, /* for platforms dependent on CPU type */
/* PCIDEP */	{0, 0, 0, 0}, /* for platforms dependent on core logic */
};

static struct platform {
  const char	   *name;
  iosys_t	    io_sys;
} platform[] = {
  {"Alcor",	IOSYS_CIA},
  {"Avanti",	IOSYS_APECS},
  {"XL",	IOSYS_APECS},
  {"Cabriolet",	IOSYS_APECS},
  {"EB164",	IOSYS_PCIDEP},
  {"EB64+",	IOSYS_APECS},
  {"EB66",	IOSYS_APECS},
  {"EB66P",	IOSYS_APECS},
  {"Jensen",	IOSYS_JENSEN},
  {"Mikasa",	IOSYS_CPUDEP},
  {"Noritake",	IOSYS_CPUDEP},
  {"Noname",	IOSYS_APECS},
  {"Sable",	IOSYS_CPUDEP},
  {"Miata",	IOSYS_CIA},
  {"Tsunami",	IOSYS_TSUNAMI},
  {"Nautilus",	IOSYS_TSUNAMI},
  {"Rawhide",	IOSYS_MCPCIA},
  {"Ruffian",	IOSYS_CIA},
  {"Takara",	IOSYS_CIA},
};

struct ioswtch {
  void		(*sethae)(unsigned long int addr);
  void		(*outb)(unsigned char b, unsigned long int port);
  void		(*outw)(unsigned short b, unsigned long int port);
  void		(*outl)(unsigned int b, unsigned long int port);
  unsigned int	(*inb)(unsigned long int port);
  unsigned int	(*inw)(unsigned long int port);
  unsigned int	(*inl)(unsigned long int port);
};

static struct {
  struct hae {
    unsigned long int	cache;
    unsigned long int *	reg;
  } hae;
  unsigned long int	base;
  struct ioswtch *	swp;
  unsigned long int	bus_memory_base;
  unsigned long int	sparse_bus_memory_base;
  unsigned long int	io_base;
  iosys_t		sys;
  ioswizzle_t		swiz;
  int			hae_shift;
} io;

extern void __sethae (unsigned long int);	/* we can't use asm/io.h */

static inline unsigned long int
port_to_cpu_addr (unsigned long int port, ioswizzle_t ioswiz, int size)
{
  if (ioswiz == IOSWIZZLE_SPARSE)
    return (port << 5) + ((size - 1) << 3) + io.base;
  else if (ioswiz == IOSWIZZLE_DENSE)
    return port + io.base;
  else
    return (port << 7) + ((size - 1) << 5) + io.base;
}

static inline void
inline_sethae (unsigned long int addr, ioswizzle_t ioswiz)
{
  if (ioswiz == IOSWIZZLE_SPARSE)
    {
      unsigned long int msb;

      /* no need to set hae if msb is 0: */
      msb = addr & 0xf8000000;
      if (msb && msb != io.hae.cache)
	{
	  __sethae (msb);
	  io.hae.cache = msb;
	}
    }
  else
    {
      /* hae on the Jensen is bits 31:25 shifted right */
      addr >>= 25;
      if (addr != io.hae.cache)
	{
	  __sethae (addr);
	  io.hae.cache = addr;
	}
    }
}

static inline void
inline_outb (unsigned char b, unsigned long int port, ioswizzle_t ioswiz)
{
  unsigned int w;
  unsigned long int addr = port_to_cpu_addr (port, ioswiz, 1);

  inline_sethae (0, ioswiz);
  asm ("insbl %2,%1,%0" : "=r" (w) : "ri" (port & 0x3), "r" (b));
  *(vuip)addr = w;
  mb ();
}


static inline void
inline_outw (unsigned short int b, unsigned long int port, ioswizzle_t ioswiz)
{
  unsigned int w;
  unsigned long int addr = port_to_cpu_addr (port, ioswiz, 2);

  inline_sethae (0, ioswiz);
  asm ("inswl %2,%1,%0" : "=r" (w) : "ri" (port & 0x3), "r" (b));
  *(vuip)addr = w;
  mb ();
}


static inline void
inline_outl (unsigned int b, unsigned long int port, ioswizzle_t ioswiz)
{
  unsigned long int addr = port_to_cpu_addr (port, ioswiz, 4);

  inline_sethae (0, ioswiz);
  *(vuip)addr = b;
  mb ();
}


static inline unsigned int
inline_inb (unsigned long int port, ioswizzle_t ioswiz)
{
  unsigned long int result, addr = port_to_cpu_addr (port, ioswiz, 1);

  inline_sethae (0, ioswiz);
  result = *(vuip) addr;
  result >>= (port & 3) * 8;
  return 0xffUL & result;
}


static inline unsigned int
inline_inw (unsigned long int port, ioswizzle_t ioswiz)
{
  unsigned long int result, addr = port_to_cpu_addr (port, ioswiz, 2);

  inline_sethae (0, ioswiz);
  result = *(vuip) addr;
  result >>= (port & 3) * 8;
  return 0xffffUL & result;
}


static inline unsigned int
inline_inl (unsigned long int port, ioswizzle_t ioswiz)
{
  unsigned long int addr = port_to_cpu_addr (port, ioswiz, 4);

  inline_sethae (0, ioswiz);
  return *(vuip) addr;
}

/*
 * Now define the inline functions for CPUs supporting byte/word insns,
 * and whose core logic supports I/O space accesses utilizing them.
 *
 * These routines could be used by MIATA, for example, because it has
 * and EV56 plus PYXIS, but it currently uses SPARSE anyway. This is
 * also true of RX164 which used POLARIS, but we will choose to use
 * these routines in that case instead of SPARSE.
 *
 * These routines are necessary for TSUNAMI/TYPHOON based platforms,
 * which will have (at least) EV6.
 */

static inline unsigned long int
dense_port_to_cpu_addr (unsigned long int port)
{
  return port + io.base;
}

static inline void
inline_bwx_outb (unsigned char b, unsigned long int port)
{
  unsigned long int addr = dense_port_to_cpu_addr (port);

  __asm__ __volatile__ ("stb %1,%0" : : "m"(*(unsigned char *)addr), "r"(b));
  mb ();
}

static inline void
inline_bwx_outw (unsigned short int b, unsigned long int port)
{
  unsigned long int addr = dense_port_to_cpu_addr (port);

  __asm__ __volatile__ ("stw %1,%0" : : "m"(*(unsigned short *)addr), "r"(b));
  mb ();
}

static inline void
inline_bwx_outl (unsigned int b, unsigned long int port)
{
  unsigned long int addr = dense_port_to_cpu_addr (port);

  *(vuip)addr = b;
  mb ();
}

static inline unsigned int
inline_bwx_inb (unsigned long int port)
{
  unsigned long int r, addr = dense_port_to_cpu_addr (port);

  __asm__ __volatile__ ("ldbu %0,%1" : "=r"(r) : "m"(*(unsigned char *)addr));
  return 0xffUL & r;
}

static inline unsigned int
inline_bwx_inw (unsigned long int port)
{
  unsigned long int r, addr = dense_port_to_cpu_addr (port);

  __asm__ __volatile__ ("ldwu %0,%1" : "=r"(r) : "m"(*(unsigned short *)addr));
  return 0xffffUL & r;
}

static inline unsigned int
inline_bwx_inl (unsigned long int port)
{
  unsigned long int addr = dense_port_to_cpu_addr (port);

  return *(vuip) addr;
}

/* macros to define routines with appropriate names and functions */

/* these do either SPARSE or JENSEN swizzle */

#define DCL_SETHAE(name, ioswiz)                        \
static void                                             \
name##_sethae (unsigned long int addr)                  \
{                                                       \
  inline_sethae (addr, IOSWIZZLE_##ioswiz);             \
}

#define DCL_OUT(name, func, type, ioswiz)		\
static void						\
name##_##func (unsigned type b, unsigned long int addr)	\
{							\
  inline_##func (b, addr, IOSWIZZLE_##ioswiz);		\
}

#define DCL_IN(name, func, ioswiz)			\
static unsigned int					\
name##_##func (unsigned long int addr)			\
{							\
  return inline_##func (addr, IOSWIZZLE_##ioswiz);	\
}

/* these do DENSE, so no swizzle is needed */

#define DCL_OUT_BWX(name, func, type)			\
static void						\
name##_##func (unsigned type b, unsigned long int addr)	\
{							\
  inline_bwx_##func (b, addr);				\
}

#define DCL_IN_BWX(name, func)				\
static unsigned int					\
name##_##func (unsigned long int addr)			\
{							\
  return inline_bwx_##func (addr);			\
}

/* now declare/define the necessary routines */

DCL_SETHAE(jensen, JENSEN)
DCL_OUT(jensen, outb, char,  JENSEN)
DCL_OUT(jensen, outw, short int, JENSEN)
DCL_OUT(jensen, outl, int,   JENSEN)
DCL_IN(jensen, inb, JENSEN)
DCL_IN(jensen, inw, JENSEN)
DCL_IN(jensen, inl, JENSEN)

DCL_SETHAE(sparse, SPARSE)
DCL_OUT(sparse, outb, char,  SPARSE)
DCL_OUT(sparse, outw, short int, SPARSE)
DCL_OUT(sparse, outl, int,   SPARSE)
DCL_IN(sparse, inb, SPARSE)
DCL_IN(sparse, inw, SPARSE)
DCL_IN(sparse, inl, SPARSE)

DCL_OUT_BWX(dense, outb, char)
DCL_OUT_BWX(dense, outw, short int)
DCL_OUT_BWX(dense, outl, int)
DCL_IN_BWX(dense, inb)
DCL_IN_BWX(dense, inw)
DCL_IN_BWX(dense, inl)

/* define the "swizzle" switch */
static struct ioswtch ioswtch[] = {
  {
    jensen_sethae,
    jensen_outb, jensen_outw, jensen_outl,
    jensen_inb, jensen_inw, jensen_inl
  },
  {
    sparse_sethae,
    sparse_outb, sparse_outw, sparse_outl,
    sparse_inb, sparse_inw, sparse_inl
  },
  {
    NULL,
    dense_outb, dense_outw, dense_outl,
    dense_inb, dense_inw, dense_inl
  }
};

#undef DEBUG_IOPERM

/* routine to process the /proc/cpuinfo information into the fields */
/* that are required for correctly determining the platform parameters */

char systype[256]; /* system type field */
char sysvari[256]; /* system variation field */
char cpumodel[256]; /* cpu model field */
int got_type, got_vari, got_model;

static int
process_cpuinfo(void)
{
  char dummy[256];
  FILE * fp;

  fp = fopen (PATH_CPUINFO, "r");
  if (!fp)
    return 0;

  got_type = got_vari = got_model = 0;
  systype[0] = sysvari[0] = cpumodel[0] = 0;

  while (1)
    {
      if (fgets (dummy, 256, fp) == NULL) break;
      /*	  fprintf(stderr, "read: %s", dummy); */
      if (!got_type &&
	  sscanf (dummy, "system type : %256[^\n]\n", systype) == 1)
	got_type = 1;
      if (!got_vari &&
	  sscanf (dummy, "system variation : %256[^\n]\n", sysvari) == 1)
	got_vari = 1;
      if (!got_model &&
	  sscanf (dummy, "cpu model : %256[^\n]\n", cpumodel) == 1)
	got_model = 1;
    }

  fclose (fp);

#ifdef DEBUG_IOPERM
  fprintf(stderr, "system type: %s\n", systype);
  fprintf(stderr, "system vari: %s\n", sysvari);
  fprintf(stderr, "cpu model: %s\n", cpumodel);
#endif

  return got_type+got_vari+got_model;
}
/*
 * Initialize I/O system.  To determine what I/O system we're dealing
 * with, we first try to read the value of symlink PATH_ALPHA_SYSTYPE,
 * if that fails, we lookup the "system type" field in /proc/cpuinfo.
 * If that fails as well, we give up.
 *
 * If the value received from PATH_ALPHA_SYSTYPE begins with a number,
 * assume this is a previously unsupported system and the values encode,
 * in order, "<io_base>,<hae_shift>,<dense_base>,<sparse_base>".
 */
static int
init_iosys (void)
{
  int i, n;

  n = readlink (PATH_ALPHA_SYSTYPE, systype, sizeof (systype) - 1);
  if (n > 0)
    {
      systype[n] = '\0';
      if (isdigit (systype[0]))
	{
	  if (sscanf (systype, "%li,%i,%li,%li", &io.io_base, &io.hae_shift,
		      &io.bus_memory_base, &io.sparse_bus_memory_base) == 4)
	    {
	      io.sys = IOSYS_UNKNOWN;
	      io.swiz = IOSWIZZLE_SPARSE;
	      io.swp = &ioswtch[IOSWIZZLE_SPARSE];
	      return 0;
	    }
	  /* else we're likely going to fail with the system match below */
	}
    }
  else
    {
      n = process_cpuinfo();

      if (!n)
	{
	  /* this can happen if the format of /proc/cpuinfo changes...  */
	  fprintf (stderr,
		   "ioperm.init_iosys(): Unable to determine system type.\n"
		   "\t(May need " PATH_ALPHA_SYSTYPE " symlink?)\n");
	  __set_errno (ENODEV);
	  return -1;
	}
    }

  /* translate systype name into i/o system: */
  for (i = 0; i < sizeof (platform) / sizeof (platform[0]); ++i)
    {
      if (strcmp (platform[i].name, systype) == 0)
	{
	  io.sys = platform[i].io_sys;
	  /* some platforms can have either EV4 or EV5 CPUs */
	  if (io.sys == IOSYS_CPUDEP) /* SABLE or MIKASA or NORITAKE so far */
	    {
	      if (strcmp (platform[i].name, "Sable") == 0)
		{
		  if (strncmp (cpumodel, "EV4", 3) == 0)
		    io.sys = IOSYS_T2;
		  else if (strncmp (cpumodel, "EV5", 3) == 0)
		    io.sys = IOSYS_GAMMA;
		}
	      else
		{ /* this covers MIKASA/NORITAKE */
		  if (strncmp (cpumodel, "EV4", 3) == 0)
		    io.sys = IOSYS_APECS;
		  else if (strncmp (cpumodel, "EV5", 3) == 0)
		    io.sys = IOSYS_CIA;
		}
	      if (io.sys == IOSYS_CPUDEP)
		{
		  /* This can happen if the format of /proc/cpuinfo changes.*/
		  fprintf (stderr, "ioperm.init_iosys(): Unable to determine"
			   " CPU model.\n");
		  __set_errno (ENODEV);
		  return -1;
		}
	    }
	  /* some platforms can have different core logic chipsets */
	  if (io.sys == IOSYS_PCIDEP) /* EB164 so far */
	    {
	      if (strcmp (systype, "EB164") == 0)
		{
		  if (strncmp (sysvari, "RX164", 5) == 0)
		    io.sys = IOSYS_POLARIS;
		  else
		    io.sys = IOSYS_CIA;
		}
	      if (io.sys == IOSYS_PCIDEP)
		{
		  /* This can happen if the format of /proc/cpuinfo changes.*/
		  fprintf (stderr, "ioperm.init_iosys(): Unable to determine"
			   " core logic chipset.\n");
		  __set_errno (ENODEV);
		  return -1;
		}
	    }
	  io.hae_shift = io_system[io.sys].hae_shift;
	  io.bus_memory_base = io_system[io.sys].bus_memory_base;
	  io.sparse_bus_memory_base = io_system[io.sys].sparse_bus_mem_base;
	  io.io_base = io_system[io.sys].bus_io_base;

	  if (io.sys == IOSYS_JENSEN)
	    io.swiz = IOSWIZZLE_JENSEN;
	  else if (io.sys == IOSYS_TSUNAMI || io.sys == IOSYS_POLARIS)
	    io.swiz = IOSWIZZLE_DENSE;
	  else
	    io.swiz = IOSWIZZLE_SPARSE;
	  io.swp = &ioswtch[io.swiz];
	  return 0;
	}
    }

  /* systype is not a know platform name... */
  __set_errno (EINVAL);
#ifdef DEBUG_IOPERM
  fprintf(stderr, "init_iosys: platform not recognized\n");
#endif
  return -1;
}


int
_ioperm (unsigned long int from, unsigned long int num, int turn_on)
{
  unsigned long int addr, len;
  int prot, err;

  if (!io.swp && init_iosys() < 0) {
#ifdef DEBUG_IOPERM
	    fprintf(stderr, "ioperm: init_iosys() failed\n");
#endif
    return -1;
  }

  /* this test isn't as silly as it may look like; consider overflows! */
  if (from >= MAX_PORT || from + num > MAX_PORT)
    {
      __set_errno (EINVAL);
#ifdef DEBUG_IOPERM
      fprintf(stderr, "ioperm: from/num out of range\n");
#endif
      return -1;
    }

#ifdef DEBUG_IOPERM
      fprintf(stderr, "ioperm: turn_on %d io.base %ld\n", turn_on, io.base);
#endif

  if (turn_on)
    {
      if (!io.base)
	{
	  int fd;

	  io.hae.reg   = 0;		/* not used in user-level */
	  io.hae.cache = 0;
	  if (io.swiz != IOSWIZZLE_DENSE)
	    __sethae (io.hae.cache);	/* synchronize with hw */

	  fd = open ("/dev/mem", O_RDWR);
	  if (fd < 0) {
#ifdef DEBUG_IOPERM
	    fprintf(stderr, "ioperm: /dev/mem open failed\n");
#endif
	    return -1;
	  }

	  addr = port_to_cpu_addr (0, io.swiz, 1);
	  len = port_to_cpu_addr (MAX_PORT, io.swiz, 1) - addr;
	  io.base =
	    (unsigned long int) __mmap (0, len, PROT_NONE, MAP_SHARED,
					fd, io.io_base);
	  close (fd);
#ifdef DEBUG_IOPERM
	  fprintf(stderr, "ioperm: mmap of len 0x%lx  returned 0x%lx\n",
		  len, io.base);
#endif
	  if ((long) io.base == -1)
	    return -1;
	}
      prot = PROT_READ | PROT_WRITE;
    }
  else
    {
      if (!io.base)
	return 0;	/* never was turned on... */

      /* turnoff access to relevant pages: */
      prot = PROT_NONE;
    }
  addr = port_to_cpu_addr (from, io.swiz, 1);
  addr &= PAGE_MASK;
  len = port_to_cpu_addr (from + num, io.swiz, 1) - addr;
  err = mprotect ((void *) addr, len, prot);
#ifdef DEBUG_IOPERM
  fprintf(stderr, "ioperm: mprotect returned %d\n", err);
#endif
  return err;
}


int
_iopl (unsigned int level)
{
    if (level > 3)
      {
	__set_errno (EINVAL);
	return -1;
      }
    if (level)
      {
	return _ioperm (0, MAX_PORT, 1);
      }
    return 0;
}


void
_sethae (unsigned long int addr)
{
  if (!io.swp && init_iosys () < 0)
    return;

  io.swp->sethae (addr);
}


void
_outb (unsigned char b, unsigned long int port)
{
  if (port >= MAX_PORT)
    return;

  io.swp->outb (b, port);
}


void
_outw (unsigned short b, unsigned long int port)
{
  if (port >= MAX_PORT)
    return;

  io.swp->outw (b, port);
}


void
_outl (unsigned int b, unsigned long int port)
{
  if (port >= MAX_PORT)
    return;

  io.swp->outl (b, port);
}


unsigned int
_inb (unsigned long int port)
{
  return io.swp->inb (port);
}


unsigned int
_inw (unsigned long int port)
{
  return io.swp->inw (port);
}


unsigned int
_inl (unsigned long int port)
{
  return io.swp->inl (port);
}


unsigned long int
_bus_base(void)
{
  if (!io.swp && init_iosys () < 0)
    return -1;
  return io.bus_memory_base;
}

unsigned long int
_bus_base_sparse(void)
{
  if (!io.swp && init_iosys () < 0)
    return -1;
  return io.sparse_bus_memory_base;
}

int
_hae_shift(void)
{
  if (!io.swp && init_iosys () < 0)
    return -1;
  return io.hae_shift;
}

weak_alias (_sethae, sethae);
weak_alias (_ioperm, ioperm);
weak_alias (_iopl, iopl);
weak_alias (_inb, inb);
weak_alias (_inw, inw);
weak_alias (_inl, inl);
weak_alias (_outb, outb);
weak_alias (_outw, outw);
weak_alias (_outl, outl);
weak_alias (_bus_base, bus_base);
weak_alias (_bus_base_sparse, bus_base_sparse);
weak_alias (_hae_shift, hae_shift);
