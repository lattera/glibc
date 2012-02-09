/* SELinux access controls for nscd.
   Copyright (C) 2004,2005,2006,2007,2009,2011 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include "config.h"
#include <error.h>
#include <errno.h>
#include <libintl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <selinux/av_permissions.h>
#include <selinux/avc.h>
#include <selinux/flask.h>
#include <selinux/selinux.h>
#ifdef HAVE_LIBAUDIT
# include <libaudit.h>
#endif

#include "dbg_log.h"
#include "selinux.h"


#ifdef HAVE_SELINUX
/* Global variable to tell if the kernel has SELinux support.  */
int selinux_enabled;

/* Define mappings of access vector permissions to request types.  */
static const access_vector_t perms[LASTREQ] =
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
  [INITGROUPS] = NSCD__GETGRP,
#ifdef NSCD__GETSERV
  [GETSERVBYNAME] = NSCD__GETSERV,
  [GETSERVBYPORT] = NSCD__GETSERV,
  [GETFDSERV] = NSCD__SHMEMSERV,
#endif
#ifdef NSCD__GETNETGRP
  [GETNETGRENT] = NSCD__GETNETGRP,
  [INNETGR] = NSCD__GETNETGRP,
  [GETFDNETGR] = NSCD__SHMEMNETGRP,
#endif
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
  if (audit_fd >= 0)
    {
      va_list ap;
      va_start (ap, fmt);

      char *buf;
      int e = vasprintf (&buf, fmt, ap);
      if (e < 0)
	{
	  buf = alloca (BUFSIZ);
	  vsnprintf (buf, BUFSIZ, fmt, ap);
	}

      /* FIXME: need to attribute this to real user, using getuid for now */
      audit_log_user_avc_message (audit_fd, AUDIT_USER_AVC, buf, NULL, NULL,
				  NULL, getuid ());

      if (e >= 0)
	free (buf);

      va_end (ap);
    }
}

/* Initialize the connection to the audit system */
static void
audit_init (void)
{
  audit_fd = audit_open ();
  if (audit_fd < 0
      /* If kernel doesn't support audit, bail out */
      && errno != EINVAL && errno != EPROTONOSUPPORT && errno != EAFNOSUPPORT)
    dbg_log (_("Failed opening connection to the audit subsystem: %m"));
}


# ifdef HAVE_LIBCAP
static const cap_value_t new_cap_list[] =
  { CAP_AUDIT_WRITE };
#  define nnew_cap_list (sizeof (new_cap_list) / sizeof (new_cap_list[0]))
static const cap_value_t tmp_cap_list[] =
  { CAP_AUDIT_WRITE, CAP_SETUID, CAP_SETGID };
#  define ntmp_cap_list (sizeof (tmp_cap_list) / sizeof (tmp_cap_list[0]))

cap_t
preserve_capabilities (void)
{
  if (getuid () != 0)
    /* Not root, then we cannot preserve anything.  */
    return NULL;

  if (prctl (PR_SET_KEEPCAPS, 1) == -1)
    {
      dbg_log (_("Failed to set keep-capabilities"));
      error (EXIT_FAILURE, errno, _("prctl(KEEPCAPS) failed"));
      /* NOTREACHED */
    }

  cap_t tmp_caps = cap_init ();
  cap_t new_caps = NULL;
  if (tmp_caps != NULL)
    new_caps = cap_init ();

  if (tmp_caps == NULL || new_caps == NULL)
    {
      if (tmp_caps != NULL)
	cap_free (tmp_caps);

      dbg_log (_("Failed to initialize drop of capabilities"));
      error (EXIT_FAILURE, 0, _("cap_init failed"));
    }

  /* There is no reason why these should not work.  */
  cap_set_flag (new_caps, CAP_PERMITTED, nnew_cap_list,
		(cap_value_t *) new_cap_list, CAP_SET);
  cap_set_flag (new_caps, CAP_EFFECTIVE, nnew_cap_list,
		(cap_value_t *) new_cap_list, CAP_SET);

  cap_set_flag (tmp_caps, CAP_PERMITTED, ntmp_cap_list,
		(cap_value_t *) tmp_cap_list, CAP_SET);
  cap_set_flag (tmp_caps, CAP_EFFECTIVE, ntmp_cap_list,
		(cap_value_t *) tmp_cap_list, CAP_SET);

  int res = cap_set_proc (tmp_caps);

  cap_free (tmp_caps);

  if (__builtin_expect (res != 0, 0))
    {
      cap_free (new_caps);
      dbg_log (_("Failed to drop capabilities"));
      error (EXIT_FAILURE, 0, _("cap_set_proc failed"));
    }

  return new_caps;
}

void
install_real_capabilities (cap_t new_caps)
{
  /* If we have no capabilities there is nothing to do here.  */
  if (new_caps == NULL)
    return;

  if (cap_set_proc (new_caps))
    {
      cap_free (new_caps);
      dbg_log (_("Failed to drop capabilities"));
      error (EXIT_FAILURE, 0, _("cap_set_proc failed"));
      /* NOTREACHED */
    }

  cap_free (new_caps);

  if (prctl (PR_SET_KEEPCAPS, 0) == -1)
    {
      dbg_log (_("Failed to unset keep-capabilities"));
      error (EXIT_FAILURE, errno, _("prctl(KEEPCAPS) failed"));
      /* NOTREACHED */
    }
}
# endif /* HAVE_LIBCAP */
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

#ifndef NSCD__GETSERV
  if (perms[req] == 0)
    {
      dbg_log (_("compile-time support for database policy missing"));
      goto out;
    }
#endif

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

#endif /* HAVE_SELINUX */
