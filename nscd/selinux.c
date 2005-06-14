/* SELinux access controls for nscd.
   Copyright (C) 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Matthew Rickard <mjricka@epoch.ncsc.mil>, 2004.

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

#include "config.h"
#include <error.h>
#include <errno.h>
#include <libintl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <selinux/av_permissions.h>
#include <selinux/avc.h>
#include <selinux/flask.h>
#include <selinux/selinux.h>
#ifdef HAVE_LIBAUDIT
#include <libaudit.h>
#endif

#include "dbg_log.h"
#include "selinux.h"


#ifdef HAVE_SELINUX
/* Global variable to tell if the kernel has SELinux support.  */
int selinux_enabled;

/* Define mappings of access vector permissions to request types.  */
static const int perms[LASTREQ] =
{
  [GETPWBYNAME] = NSCD__GETPWD,
  [GETPWBYUID] = NSCD__GETPWD,
  [GETGRBYNAME] = NSCD__GETGRP,
  [GETGRBYGID] = NSCD__GETGRP,
  [GETHOSTBYNAME] = NSCD__GETHOST,
  [GETHOSTBYNAMEv6] = NSCD__GETHOST,
  [GETHOSTBYADDR] = NSCD__GETHOST,
  [GETHOSTBYADDRv6] = NSCD__GETHOST,
  [GETSTAT] = NSCD__GETSTAT,
  [SHUTDOWN] = NSCD__ADMIN,
  [INVALIDATE] = NSCD__ADMIN,
  [GETFDPW] = NSCD__SHMEMPWD,
  [GETFDGR] = NSCD__SHMEMGRP,
  [GETFDHST] = NSCD__SHMEMHOST,
  [GETAI] = NSCD__GETHOST,
  [INITGROUPS] = NSCD__GETGRP
};

/* Store an entry ref to speed AVC decisions.  */
static struct avc_entry_ref aeref;

/* Thread to listen for SELinux status changes via netlink.  */
static pthread_t avc_notify_thread;

#ifdef HAVE_LIBAUDIT
/* Prototype for supporting the audit daemon */
static void log_callback (const char *fmt, ...);
#endif

/* Prototypes for AVC callback functions.  */
static void *avc_create_thread (void (*run) (void));
static void avc_stop_thread (void *thread);
static void *avc_alloc_lock (void);
static void avc_get_lock (void *lock);
static void avc_release_lock (void *lock);
static void avc_free_lock (void *lock);

/* AVC callback structures for use in avc_init.  */
static const struct avc_log_callback log_cb =
{
#ifdef HAVE_LIBAUDIT
  .func_log = log_callback,
#else
  .func_log = dbg_log,
#endif
  .func_audit = NULL
};
static const struct avc_thread_callback thread_cb =
{
  .func_create_thread = avc_create_thread,
  .func_stop_thread = avc_stop_thread
};
static const struct avc_lock_callback lock_cb =
{
  .func_alloc_lock = avc_alloc_lock,
  .func_get_lock = avc_get_lock,
  .func_release_lock = avc_release_lock,
  .func_free_lock = avc_free_lock
};

#ifdef HAVE_LIBAUDIT
/* The audit system's netlink socket descriptor */
static int audit_fd = -1;

/* When an avc denial occurs, log it to audit system */
static void 
log_callback (const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  audit_log_avc (audit_fd, AUDIT_USER_AVC, fmt, ap);
  va_end (ap);
}

/* Initialize the connection to the audit system */
static void 
audit_init (void)
{
  audit_fd = audit_open ();
  if (audit_fd < 0)
     dbg_log (_("Failed opening connection to the audit subsystem"));
}
#endif /* HAVE_LIBAUDIT */

/* Determine if we are running on an SELinux kernel. Set selinux_enabled
   to the result.  */
void
nscd_selinux_enabled (int *selinux_enabled)
{
  *selinux_enabled = is_selinux_enabled ();
  if (*selinux_enabled < 0)
    {
      dbg_log (_("Failed to determine if kernel supports SELinux"));
      exit (EXIT_FAILURE);
    }
}


/* Create thread for AVC netlink notification.  */
static void *
avc_create_thread (void (*run) (void))
{
  int rc;

  rc =
    pthread_create (&avc_notify_thread, NULL, (void *(*) (void *)) run, NULL);
  if (rc != 0)
    error (EXIT_FAILURE, rc, _("Failed to start AVC thread"));

  return &avc_notify_thread;
}


/* Stop AVC netlink thread.  */
static void
avc_stop_thread (void *thread)
{
  pthread_cancel (*(pthread_t *) thread);
}


/* Allocate a new AVC lock.  */
static void *
avc_alloc_lock (void)
{
  pthread_mutex_t *avc_mutex;

  avc_mutex = malloc (sizeof (pthread_mutex_t));
  if (avc_mutex == NULL)
    error (EXIT_FAILURE, errno, _("Failed to create AVC lock"));
  pthread_mutex_init (avc_mutex, NULL);

  return avc_mutex;
}


/* Acquire an AVC lock.  */
static void
avc_get_lock (void *lock)
{
  pthread_mutex_lock (lock);
}


/* Release an AVC lock.  */
static void
avc_release_lock (void *lock)
{
  pthread_mutex_unlock (lock);
}


/* Free an AVC lock.  */
static void
avc_free_lock (void *lock)
{
  pthread_mutex_destroy (lock);
  free (lock);
}


/* Initialize the user space access vector cache (AVC) for NSCD along with
   log/thread/lock callbacks.  */
void
nscd_avc_init (void)
{
  avc_entry_ref_init (&aeref);

  if (avc_init ("avc", NULL, &log_cb, &thread_cb, &lock_cb) < 0)
    error (EXIT_FAILURE, errno, _("Failed to start AVC"));
  else
    dbg_log (_("Access Vector Cache (AVC) started"));
#ifdef HAVE_LIBAUDIT
  audit_init ();
#endif
}


/* Check the permission from the caller (via getpeercon) to nscd.
   Returns 0 if access is allowed, 1 if denied, and -1 on error.  */
int
nscd_request_avc_has_perm (int fd, request_type req)
{
  /* Initialize to NULL so we know what to free in case of failure.  */
  security_context_t scon = NULL;
  security_context_t tcon = NULL;
  security_id_t ssid = NULL;
  security_id_t tsid = NULL;
  int rc = -1;

  if (getpeercon (fd, &scon) < 0)
    {
      dbg_log (_("Error getting context of socket peer"));
      goto out;
    }
  if (getcon (&tcon) < 0)
    {
      dbg_log (_("Error getting context of nscd"));
      goto out;
    }
  if (avc_context_to_sid (scon, &ssid) < 0
      || avc_context_to_sid (tcon, &tsid) < 0)
    {
      dbg_log (_("Error getting sid from context"));
      goto out;
    }

  rc = avc_has_perm (ssid, tsid, SECCLASS_NSCD, perms[req], &aeref, NULL) < 0;

out:
  if (scon)
    freecon (scon);
  if (tcon)
    freecon (tcon);
  if (ssid)
    sidput (ssid);
  if (tsid)
    sidput (tsid);

  return rc;
}


/* Wrapper to get AVC statistics.  */
void
nscd_avc_cache_stats (struct avc_cache_stats *cstats)
{
  avc_cache_stats (cstats);
}


/* Print the AVC statistics to stdout.  */
void
nscd_avc_print_stats (struct avc_cache_stats *cstats)
{
  printf (_("\nSELinux AVC Statistics:\n\n"
	    "%15u  entry lookups\n"
	    "%15u  entry hits\n"
	    "%15u  entry misses\n"
	    "%15u  entry discards\n"
	    "%15u  CAV lookups\n"
	    "%15u  CAV hits\n"
	    "%15u  CAV probes\n"
	    "%15u  CAV misses\n"),
	  cstats->entry_lookups, cstats->entry_hits, cstats->entry_misses,
	  cstats->entry_discards, cstats->cav_lookups, cstats->cav_hits,
	  cstats->cav_probes, cstats->cav_misses);
}


/* Clean up the AVC before exiting.  */
void
nscd_avc_destroy (void)
{
  avc_destroy ();
#ifdef HAVE_LIBAUDIT
  audit_close (audit_fd);
#endif
}

#endif /* HAVE_SELINUX */
