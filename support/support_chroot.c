/* Setup a chroot environment for use within tests.
   Copyright (C) 2017 Free Software Foundation, Inc.
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

#include <stdlib.h>
#include <support/check.h>
#include <support/namespace.h>
#include <support/support.h>
#include <support/temp_file.h>
#include <support/test-driver.h>
#include <support/xunistd.h>

struct support_chroot *
support_chroot_create (struct support_chroot_configuration conf)
{
  struct support_chroot *chroot = xmalloc (sizeof (*chroot));

  chroot->path_chroot = xasprintf ("%s/tst-resolv-res_init-XXXXXX", test_dir);
  if (mkdtemp (chroot->path_chroot) == NULL)
    FAIL_EXIT1 ("mkdtemp (\"%s\"): %m", chroot->path_chroot);
  add_temp_file (chroot->path_chroot);

  /* Create the /etc directory in the chroot environment.  */
  char *path_etc = xasprintf ("%s/etc", chroot->path_chroot);
  xmkdir (path_etc, 0777);
  add_temp_file (path_etc);

  if (conf.resolv_conf != NULL)
    {
      /* Create an empty resolv.conf file.  */
      chroot->path_resolv_conf = xasprintf ("%s/resolv.conf", path_etc);
      add_temp_file (chroot->path_resolv_conf);
      support_write_file_string (chroot->path_resolv_conf, conf.resolv_conf);
    }
  else
    chroot->path_resolv_conf = NULL;

  free (path_etc);

  /* valgrind needs a temporary directory in the chroot.  */
  {
    char *path_tmp = xasprintf ("%s/tmp", chroot->path_chroot);
    xmkdir (path_tmp, 0777);
    add_temp_file (path_tmp);
    free (path_tmp);
  }

  return chroot;
}

void
support_chroot_free (struct support_chroot *chroot)
{
  free (chroot->path_chroot);
  free (chroot->path_resolv_conf);
  free (chroot);
}
