/* Profiling of shared libraries.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <link.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/gmon.h>
#include <sys/gmon_out.h>
#include <sys/mman.h>
#include <sys/stat.h>

/* The LD_PROFILE feature has to be implemented different to the
   normal profiling using the gmon/ functions.  The problem is that an
   arbitrary amount of processes simulataneously can be run using
   profiling and all write the results in the same file.  To provide
   this mechanism one could implement a complicated mechanism to merge
   the content of two profiling runs or one could extend the file
   format to allow more than one data set.  For the second solution we
   would have the problem that the file can grow in size beyond any
   limit and both solutions have the problem that the concurrency of
   writing the results is a big problem.

   Another much simpler method is to use mmap to map the same file in
   all using programs and modify the data in the mmap'ed area and so
   also automatically on the disk.  Using the MAP_SHARED option of
   mmap(2) this can be done without big problems in more than one
   file.

   This approach is very different from the normal profiling.  We have
   to use the profiling data in exactly the way they are expected to
   be written to disk.  */

extern char *_strerror_internal __P ((int, char *buf, size_t));

extern int __profile_frequency __P ((void));


static struct gmonparam param;

/* We define a special type to address the elements of the arc table.
   This is basically the `gmon_cg_arc_record' format but it includes
   the room for the tag and it uses real types.  */
struct here_cg_arc_record
  {
    char tag;
    uintptr_t from_pc __attribute__ ((packed));
    uintptr_t self_pc __attribute__ ((packed));
    uint32_t count __attribute__ ((packed));
  };

static struct here_cg_arc_record *data;


void
_dl_start_profile (struct link_map *map, const char *output_dir)
{
  char *filename;
  int fd;
  struct stat st;
  const ElfW(Phdr) *ph;
  ElfW(Addr) mapstart = ~((ElfW(Addr)) 0);
  ElfW(Addr) mapend = 0;
  off_t expected_size;
  struct gmon_hdr gmon_hdr;
  struct gmon_hist_hdr hist_hdr;
  struct gmon_hdr *addr;
  char *hist;

  /* Compute the size of the sections which contain program code.  */
  for (ph = map->l_phdr; ph < &map->l_phdr[map->l_phnum]; ++ph)
    if (ph->p_type == PT_LOAD && (ph->p_flags & PF_X))
      {
	ElfW(Addr) start = (ph->p_vaddr & ~(_dl_pagesize - 1));
	ElfW(Addr) end = ((ph->p_vaddr + ph->p_memsz + _dl_pagesize - 1)
			  & ~(_dl_pagesize - 1));

	if (start < mapstart)
	  mapstart = start;
	if (end > mapend)
	  mapend = end;
      }

  /* Now we can compute the size of the profiling data.  This is done
     with the same formulars as in `monstartup' (see gmon.c).  */
  param.state = GMON_PROF_OFF;
  param.lowpc = mapstart + map->l_addr;
  param.highpc = mapend + map->l_addr;
  param.textsize = mapend - mapstart;
  param.kcountsize = param.textsize / HISTFRACTION;
  param.hashfraction = HASHFRACTION;
  param.log_hashfraction = -1;
  if ((HASHFRACTION & (HASHFRACTION - 1)) == 0)
    /* If HASHFRACTION is a power of two, mcount can use shifting
       instead of integer division.  Precompute shift amount.  */
    param.log_hashfraction = ffs (param.hashfraction
				  * sizeof (*param.froms)) - 1;
  param.fromssize = param.textsize / HASHFRACTION;
  param.tolimit = param.textsize * ARCDENSITY / 100;
  if (param.tolimit < MINARCS)
    param.tolimit = MINARCS;
  if (param.tolimit > MAXARCS)
    param.tolimit = MAXARCS;
  param.tossize = param.tolimit * sizeof (struct tostruct);

  expected_size = (sizeof (struct gmon_hdr)
		   + 1 + sizeof (struct gmon_hist_hdr)
		   + ((1 + sizeof (struct gmon_cg_arc_record))
		      * (param.fromssize / sizeof (*param.froms))));

  /* Create the gmon_hdr we expect or write.  */
  memset (&gmon_hdr, '\0', sizeof (struct gmon_hdr));
  memcpy (&gmon_hdr.cookie[0], GMON_MAGIC, sizeof (gmon_hdr.cookie));
  *(int32_t *) gmon_hdr.version = GMON_VERSION;

  /* Create the hist_hdr we expect or write.  */
  *(char **) hist_hdr.low_pc = (char *) mapstart;
  *(char **) hist_hdr.high_pc = (char *) mapend;
  *(int32_t *) hist_hdr.hist_size = param.kcountsize / sizeof (HISTCOUNTER);
  *(int32_t *) hist_hdr.prof_rate = __profile_frequency ();
  strncpy (hist_hdr.dimen, "seconds", sizeof (hist_hdr.dimen));
  hist_hdr.dimen_abbrev = 's';

  /* First determine the output name.  We write in the directory
     OUTPUT_DIR and the name is composed from the shared objects
     soname (or the file name) and the ending ".profile".  */
  filename = (char *) alloca (strlen (output_dir) + 1 + strlen (_dl_profile)
			      + sizeof ".profile");
  __stpcpy (__stpcpy (__stpcpy (__stpcpy (filename, output_dir), "/"),
		      _dl_profile),
	    ".profile");

  fd = __open (filename, O_RDWR | O_CREAT, 0666);
  if (fd == -1)
    /* We cannot write the profiling data so don't do anthing.  */
    return;

  if (fstat (fd, &st) < 0 || !S_ISREG (st.st_mode))
    {
      /* Not stat'able or not a regular file => don't use it.  */
      close (fd);
      return;
    }

  /* Test the size.  If it does not match what we expect from the size
     values in the map MAP we don't use it and warn the user.  */
  if (st.st_size == 0)
    {
      /* We have to create the file.  */
      char buf[_dl_pagesize];

      memset (buf, '\0', _dl_pagesize);

      if (__lseek (fd, expected_size & ~(_dl_pagesize - 1), SEEK_SET) == -1)
	{
	  char buf[400];
	  int errnum;
	cannot_create:
	  errnum = errno;
	  __close (fd);
	  fprintf (stderr, "%s: cannot create file: %s\n", filename,
		   _strerror_internal (errnum, buf, sizeof buf));
	  return;
	}

      if (TEMP_FAILURE_RETRY (__write (fd, buf, (expected_size
						 & (_dl_pagesize - 1)))) < 0)
	goto cannot_create;
    }
  else if (st.st_size != expected_size)
    {
      __close (fd);
    wrong_format:
      fprintf (stderr, "%s: file is no correct profile data file for `%s'\n",
	       filename, _dl_profile);
      return;
    }

  addr = (void *) __mmap (NULL, expected_size, PROT_READ|PROT_WRITE,
			  MAP_SHARED|MAP_FILE, fd, 0);
  if (addr == (void *) -1)
    {
      char buf[400];
      int errnum = errno;
      __close (fd);
      fprintf (stderr, "%s: cannot map file: %s\n", filename,
	       _strerror_internal (errnum, buf, sizeof buf));
      return;
    }

  /* We don't need the file desriptor anymore.  */
  __close (fd);

  /* Pointer to data after the header.  */
  hist = (char *) (addr + 1);

  /* Compute pointer to array of the arc information.  */
  data = (struct here_cg_arc_record *) (hist + 1
					+ sizeof (struct gmon_hist_hdr));

  if (st.st_size == 0)
    {
      /* Create the signature.  */
      size_t cnt;

      memcpy (addr, &gmon_hdr, sizeof (struct gmon_hdr));

      *hist = GMON_TAG_TIME_HIST;
      memcpy (hist + 1, &hist_hdr, sizeof (struct gmon_hist_hdr));

      for (cnt = 0; cnt < param.fromssize / sizeof (*param.froms); ++cnt)
	data[cnt].tag = GMON_TAG_CG_ARC;
    }
  else
    {
      /* Test the signature in the file.  */
      if (memcmp (addr, &gmon_hdr, sizeof (struct gmon_hdr)) != 0
	  || *hist != GMON_TAG_TIME_HIST
	  || memcmp (hist + 1, &hist_hdr, sizeof (struct gmon_hist_hdr)) != 0)
	goto wrong_format;
    }

  /* Turn on profiling.  */
  param.state = GMON_PROF_ON;
}


void
_dl_mcount (ElfW(Addr) frompc, ElfW(Addr) selfpc)
{
  if (param.state != GMON_PROF_ON)
    return;
  param.state = GMON_PROF_BUSY;

  /* Compute relative addresses.  The shared object can be loaded at
     any address.  The value of frompc could be anything.  We cannot
     restrict it in any way, just set to a fixed value (0) in case it
     is outside the allowed range.  These calls show up as calls from
     <external> in the gprof output.  */
  frompc -= param.lowpc;
  if (frompc >= param.textsize)
    frompc = 0;
  selfpc -= param.lowpc;

  param.state = GMON_PROF_ON;
}
