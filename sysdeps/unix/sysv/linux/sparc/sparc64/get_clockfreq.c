/* Get frequency of the system processor.  sparc64 version.
   Copyright (C) 2001-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <libc-internal.h>
#include <asm/openpromio.h>

static hp_timing_t
__get_clockfreq_via_cpuinfo (void)
{
  hp_timing_t result;
  int fd;

  result = 0;

  fd = __open ("/proc/cpuinfo", O_RDONLY);
  if (fd != -1)
    {
      char buf[8192];
      ssize_t n;

      n = __read (fd, buf, sizeof buf);
      if (n > 0)
	{
	  char *mhz = memmem (buf, n, "Cpu0ClkTck", 7);

	  if (mhz != NULL)
	    {
	      char *endp = buf + n;

	      /* Search for the beginning of the string.  */
	      while (mhz < endp
		     && (*mhz < '0' || *mhz > '9')
		     && (*mhz < 'a' || *mhz > 'f')
		     && *mhz != '\n')
		++mhz;

	      while (mhz < endp && *mhz != '\n')
		{
		  if ((*mhz >= '0' && *mhz <= '9') ||
		      (*mhz >= 'a' && *mhz <= 'f'))
		    {
		      result <<= 4;
		      if (*mhz >= '0' && *mhz <= '9')
			result += *mhz - '0';
		      else
			result += (*mhz - 'a') + 10;
		    }
		  ++mhz;
		}
	    }
	}

      __close (fd);
    }

  return result;
}

static hp_timing_t
__get_clockfreq_via_proc_openprom (void)
{
  hp_timing_t result;
  int obp_fd;

  result = 0;

  obp_fd = __open ("/proc/openprom", O_RDONLY);
  if (obp_fd != -1)
    {
      unsigned long int buf[4096 / sizeof (unsigned long int)];
      struct dirent *dirp = (struct dirent *) buf;
      ssize_t len;

      while ((len = __getdents (obp_fd, (char *) dirp, sizeof (buf))) > 0)
	{
	  struct dirent *this_dirp = dirp;

	  while (len > 0)
	    {
	      char node[strlen ("/proc/openprom/")
			+ _D_ALLOC_NAMLEN (this_dirp)
			+ strlen ("/clock-frequency")];
	      char *prop;
	      int fd;

	      /* Note that
		   strlen("/clock-frequency") > strlen("/device_type")
	      */
	      __stpcpy (prop = __stpcpy (__stpcpy (node, "/proc/openprom/"),
					 this_dirp->d_name),
			"/device_type");
	      fd = __open (node, O_RDONLY);
	      if (fd != -1)
		{
		  char type_string[128];
		  int ret;

		  ret = __read (fd, type_string, sizeof (type_string));
		  if (ret > 0 && strncmp (type_string, "'cpu'", 5) == 0)
		    {
		      int clkfreq_fd;

		      __stpcpy (prop, "/clock-frequency");
		      clkfreq_fd = __open (node, O_RDONLY);
		      if (clkfreq_fd != -1)
			{
			  if (__read (clkfreq_fd, type_string,
				      sizeof (type_string)) > 0)
			    result = (hp_timing_t)
			      strtoumax (type_string, NULL, 16);
			  __close (clkfreq_fd);
			}
		    }
		  __close (fd);
		}

	      if (result != 0)
		break;

	      len -= this_dirp->d_reclen;
	      this_dirp = (struct dirent *)
		((char *) this_dirp + this_dirp->d_reclen);
	    }
	  if (result != 0)
	    break;
	}
      __close (obp_fd);
    }

  return result;
}

static void set_obp_int (struct openpromio *op, int val)
{
  char *cp = op->oprom_array;
  int *ip = (int *) cp;

  *ip = val;
}

static int get_obp_int (struct openpromio *op)
{
  char *cp = op->oprom_array;
  int *ip = (int *) cp;

  return *ip;
}

static hp_timing_t
__get_clockfreq_via_dev_openprom (void)
{
  hp_timing_t result;
  int obp_dev_fd;

  result = 0;

  obp_dev_fd = __open ("/dev/openprom", O_RDONLY);
  if (obp_dev_fd != -1)
    {
      char obp_buf[8192];
      struct openpromio *obp_cmd = (struct openpromio *)obp_buf;
      int ret;

      obp_cmd->oprom_size =
	sizeof (obp_buf) - sizeof (unsigned int);
      set_obp_int (obp_cmd, 0);
      ret = __ioctl (obp_dev_fd, OPROMCHILD, (char *) obp_cmd);
      if (ret == 0)
	{
	  int cur_node = get_obp_int (obp_cmd);

	  while (cur_node != 0 && cur_node != -1)
	    {
	      obp_cmd->oprom_size = sizeof (obp_buf) - sizeof (unsigned int);
	      strcpy (obp_cmd->oprom_array, "device_type");
	      ret = __ioctl (obp_dev_fd, OPROMGETPROP, (char *) obp_cmd);
	      if (ret == 0
		  && strncmp (obp_cmd->oprom_array, "cpu", 3) == 0)
		{
		  obp_cmd->oprom_size = (sizeof (obp_buf)
					 - sizeof (unsigned int));
		  strcpy (obp_cmd->oprom_array, "clock-frequency");
		  ret = __ioctl (obp_dev_fd, OPROMGETPROP, (char *) obp_cmd);
		  if (ret == 0)
		    result = (hp_timing_t) get_obp_int (obp_cmd);
		}
	      obp_cmd->oprom_size = sizeof (obp_buf) - sizeof (unsigned int);
	      set_obp_int (obp_cmd, cur_node);
	      ret = __ioctl (obp_dev_fd, OPROMNEXT, (char *) obp_cmd);
	      if (ret < 0)
		break;
	      cur_node = get_obp_int (obp_cmd);
	    }
	}
    }

  return result;
}

hp_timing_t
__get_clockfreq (void)
{
  static hp_timing_t result;

  /* If this function was called before, we know the result.  */
  if (result != 0)
    return result;

  /* We first read the information from the /proc/cpuinfo file.
     It contains at least one line like
	Cpu0ClkTick         : 000000002cb41780
     We search for this line and convert the number in an integer.  */
  result = __get_clockfreq_via_cpuinfo ();
  if (result != 0)
    return result;

  /* If that did not work, try to find an OpenPROM node
     with device_type equal to 'cpu' using /dev/openprom
     and fetch the clock-frequency property from there.  */
  result = __get_clockfreq_via_dev_openprom ();
  if (result != 0)
    return result;

  /* Finally, try the same lookup as above but using /proc/openprom.  */
  result = __get_clockfreq_via_proc_openprom ();

  return result;
}
