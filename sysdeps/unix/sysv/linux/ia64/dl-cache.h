/* Support for reading /etc/ld.so.cache files written by Linux ldconfig.
   Copyright (C) 2000 Free Software Foundation, Inc.
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

#define _DL_CACHE_DEFAULT_ID	0x203

#define _dl_cache_check_flags(flags) \
  ((flags) == _DL_CACHE_DEFAULT_ID)

#define EMUL_HACK "/emul/ia32-linux"

#define arch_startup(argc, argv) unlink (EMUL_HACK LD_SO_CACHE)

#define add_arch_dirs(config_file) \
  do {							\
    int save_verbose = opt_verbose;			\
    opt_verbose = 0;					\
							\
    parse_conf (config_file, EMUL_HACK, true);		\
							\
    /* Always add the standard search paths.  */	\
    add_system_dir (EMUL_HACK SLIBDIR);			\
    if (strcmp (SLIBDIR, LIBDIR))			\
      add_system_dir (EMUL_HACK LIBDIR);		\
							\
    char emul_config_file[strlen (config_file)		\
			  + sizeof EMUL_HACK];		\
    strcpy (mempcpy (emul_config_file, EMUL_HACK,	\
		     strlen (EMUL_HACK)), config_file);	\
							\
    if (! access (emul_config_file, R_OK))		\
      parse_conf (emul_config_file, EMUL_HACK, true);	\
							\
    opt_verbose = save_verbose;				\
  } while (0)

#include_next <dl-cache.h>
